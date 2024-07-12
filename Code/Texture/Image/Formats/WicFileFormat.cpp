#include <Texture/TexturePCH.h>

#include <Foundation/Basics/Platform/Win/HResultUtils.h>
#include <Foundation/Containers/StaticArray.h>
#include <Foundation/IO/Stream.h>
#include <Texture/Image/Formats/ImageFormatMappings.h>
#include <Texture/Image/Formats/WicFileFormat.h>
#include <Texture/Image/Image.h>
#include <Texture/Image/ImageConversion.h>

#if NS_ENABLED(NS_PLATFORM_WINDOWS_DESKTOP)

#  include <Foundation/IO/StreamUtils.h>
#  include <Foundation/Profiling/Profiling.h>
#  include <Texture/DirectXTex/DirectXTex.h>

using namespace DirectX;

NS_DEFINE_AS_POD_TYPE(DirectX::Image); // Allow for storing this struct in ns containers

// NS_STATICLINK_FORCE
nsWicFileFormat g_wicFormat;

namespace
{
  /// \brief Try to init COM, return true if we are the first(!) to successfully do so
  bool InitializeCOM()
  {
    HRESULT result = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (result == S_OK)
    {
      // We were the first one - deinit on shutdown
      return true;
    }
    else if (SUCCEEDED(result))
    {
      // We were not the first one, but we still succeeded, so we deinit COM right away.
      // Otherwise we might be the last one to call CoUninitialize(), but that is supposed to be the one who called
      // CoInitialize[Ex]() first.
      CoUninitialize();
    }

    // We won't call CoUninitialize() on shutdown as either we were not the first one to init COM successfully (and uninitialized it right away),
    // or our call to CoInitializeEx() didn't succeed, because it was already called with another concurrency model specifier.
    return false;
  }
} // namespace


nsWicFileFormat::nsWicFileFormat() = default;

nsWicFileFormat::~nsWicFileFormat()
{
  if (m_bCoUninitOnShutdown)
  {
    // We were the first one to successfully initialize COM, so we are the one who needs to shut it down.
    CoUninitialize();
  }
}

nsResult nsWicFileFormat::ReadFileData(nsStreamReader& stream, nsDynamicArray<nsUInt8>& storage) const
{
  if (m_bTryCoInit)
  {
    m_bCoUninitOnShutdown = InitializeCOM();
    m_bTryCoInit = false;
  }

  nsStreamUtils::ReadAllAndAppend(stream, storage);

  if (storage.IsEmpty())
  {
    nsLog::Error("Failure to retrieve image data.");
    return NS_FAILURE;
  }

  return NS_SUCCESS;
}

static void SetHeader(nsImageHeader& ref_header, nsImageFormat::Enum imageFormat, const TexMetadata& metadata)
{
  ref_header.SetImageFormat(imageFormat);

  ref_header.SetWidth(nsUInt32(metadata.width));
  ref_header.SetHeight(nsUInt32(metadata.height));
  ref_header.SetDepth(nsUInt32(metadata.depth));

  ref_header.SetNumMipLevels(1);
  ref_header.SetNumArrayIndices(nsUInt32(metadata.IsCubemap() ? (metadata.arraySize / 6) : metadata.arraySize));
  ref_header.SetNumFaces(metadata.IsCubemap() ? 6 : 1);
}

nsResult nsWicFileFormat::ReadImageHeader(nsStreamReader& inout_stream, nsImageHeader& ref_header, nsStringView sFileExtension) const
{
  NS_PROFILE_SCOPE("nsWicFileFormat::ReadImageHeader");

  nsDynamicArray<nsUInt8> storage;
  NS_SUCCEED_OR_RETURN(ReadFileData(inout_stream, storage));

  TexMetadata metadata;
  ScratchImage scratchImage;
  WIC_FLAGS wicFlags = WIC_FLAGS_ALL_FRAMES | WIC_FLAGS_IGNORE_SRGB /* just treat PNG, JPG etc as non-sRGB, we determine this through our 'Usage' later */;

  HRESULT loadResult = GetMetadataFromWICMemory(storage.GetData(), storage.GetCount(), wicFlags, metadata);
  if (FAILED(loadResult))
  {
    nsLog::Error("Failure to load image metadata. HRESULT:{}", nsArgErrorCode(loadResult));
    return NS_FAILURE;
  }

  nsImageFormat::Enum imageFormat = nsImageFormatMappings::FromDxgiFormat(metadata.format);

  if (imageFormat == nsImageFormat::UNKNOWN)
  {
    nsLog::Warning("Unable to use image format from '{}' file - trying conversion.", sFileExtension);
    wicFlags |= WIC_FLAGS_FORCE_RGB;
    GetMetadataFromWICMemory(storage.GetData(), storage.GetCount(), wicFlags, metadata);
    imageFormat = nsImageFormatMappings::FromDxgiFormat(metadata.format);
  }

  if (imageFormat == nsImageFormat::UNKNOWN)
  {
    nsLog::Error("Unable to use image format from '{}' file.", sFileExtension);
    return NS_FAILURE;
  }

  SetHeader(ref_header, imageFormat, metadata);

  return NS_SUCCESS;
}

nsResult nsWicFileFormat::ReadImage(nsStreamReader& inout_stream, nsImage& ref_image, nsStringView sFileExtension) const
{
  NS_PROFILE_SCOPE("nsWicFileFormat::ReadImage");

  nsDynamicArray<nsUInt8> storage;
  NS_SUCCEED_OR_RETURN(ReadFileData(inout_stream, storage));

  TexMetadata metadata;
  ScratchImage scratchImage;
  WIC_FLAGS wicFlags = WIC_FLAGS_ALL_FRAMES | WIC_FLAGS_IGNORE_SRGB /* just treat PNG, JPG etc as non-sRGB, we determine this through our 'Usage' later */;

  // Read WIC data from local storage
  HRESULT loadResult = LoadFromWICMemory(storage.GetData(), storage.GetCount(), wicFlags, nullptr, scratchImage);
  if (FAILED(loadResult))
  {
    nsLog::Error("Failure to load image data. HRESULT:{}", nsArgErrorCode(loadResult));
    return NS_FAILURE;
  }

  // Determine image format, re-reading image data if necessary
  metadata = scratchImage.GetMetadata();

  nsImageFormat::Enum imageFormat = nsImageFormatMappings::FromDxgiFormat(metadata.format);

  if (imageFormat == nsImageFormat::UNKNOWN)
  {
    nsLog::Warning("Unable to use image format from '{}' file - trying conversion.", sFileExtension);
    wicFlags |= WIC_FLAGS_FORCE_RGB;
    LoadFromWICMemory(storage.GetData(), storage.GetCount(), wicFlags, nullptr, scratchImage);
    imageFormat = nsImageFormatMappings::FromDxgiFormat(metadata.format);
  }

  if (imageFormat == nsImageFormat::UNKNOWN)
  {
    nsLog::Error("Unable to use image format from '{}' file.", sFileExtension);
    return NS_FAILURE;
  }

  // Prepare destination image header and allocate storage
  nsImageHeader imageHeader;
  SetHeader(imageHeader, imageFormat, metadata);

  ref_image.ResetAndAlloc(imageHeader);

  // Read image data into destination image
  nsUInt64 destRowPitch = imageHeader.GetRowPitch();
  nsUInt32 itemIdx = 0;
  for (nsUInt32 arrayIdx = 0; arrayIdx < imageHeader.GetNumArrayIndices(); ++arrayIdx)
  {
    for (nsUInt32 faceIdx = 0; faceIdx < imageHeader.GetNumFaces(); ++faceIdx, ++itemIdx)
    {
      for (nsUInt32 sliceIdx = 0; sliceIdx < imageHeader.GetDepth(); ++sliceIdx)
      {
        const Image* sourceImage = scratchImage.GetImage(0, itemIdx, sliceIdx);
        nsUInt8* destPixels = ref_image.GetPixelPointer<nsUInt8>(0, faceIdx, arrayIdx, 0, 0, sliceIdx);

        if (sourceImage && destPixels && sourceImage->pixels)
        {
          if (destRowPitch == sourceImage->rowPitch)
          {
            // Fast path: Just copy the entire thing
            nsMemoryUtils::Copy(destPixels, sourceImage->pixels, static_cast<size_t>(imageHeader.GetHeight() * destRowPitch));
          }
          else
          {
            // Row pitches don't match - copy row by row
            nsUInt64 bytesPerRow = nsMath::Min(destRowPitch, nsUInt64(sourceImage->rowPitch));
            const uint8_t* sourcePixels = sourceImage->pixels;
            for (nsUInt32 rowIdx = 0; rowIdx < imageHeader.GetHeight(); ++rowIdx)
            {
              nsMemoryUtils::Copy(destPixels, sourcePixels, static_cast<size_t>(bytesPerRow));

              destPixels += destRowPitch;
              sourcePixels += sourceImage->rowPitch;
            }
          }
        }
      }
    }
  }

  return NS_SUCCESS;
}

nsResult nsWicFileFormat::WriteImage(nsStreamWriter& inout_stream, const nsImageView& image, nsStringView sFileExtension) const
{
  if (m_bTryCoInit)
  {
    m_bCoUninitOnShutdown = InitializeCOM();
    m_bTryCoInit = false;
  }

  using namespace DirectX;

  // Convert into suitable output format
  nsImageFormat::Enum compatibleFormats[] = {
    nsImageFormat::R8G8B8A8_UNORM,
    nsImageFormat::R8G8B8A8_UNORM_SRGB,
    nsImageFormat::R8_UNORM,
    nsImageFormat::R16G16B16A16_UNORM,
    nsImageFormat::R16_UNORM,
    nsImageFormat::R32G32B32A32_FLOAT,
    nsImageFormat::R32G32B32_FLOAT,
  };

  // Find a compatible format closest to the one the image currently has
  nsImageFormat::Enum format = nsImageConversion::FindClosestCompatibleFormat(image.GetImageFormat(), compatibleFormats);

  if (format == nsImageFormat::UNKNOWN)
  {
    nsLog::Error("No conversion from format '{0}' to a format suitable for '{}' files known.", nsImageFormat::GetName(image.GetImageFormat()), sFileExtension);
    return NS_FAILURE;
  }

  // Convert if not already in a compatible format
  if (format != image.GetImageFormat())
  {
    nsImage convertedImage;
    if (nsImageConversion::Convert(image, convertedImage, format) != NS_SUCCESS)
    {
      // This should never happen
      NS_ASSERT_DEV(false, "nsImageConversion::Convert failed even though the conversion was to the format returned by FindClosestCompatibleFormat.");
      return NS_FAILURE;
    }

    return WriteImage(inout_stream, convertedImage, sFileExtension);
  }

  // Store nsImage data in DirectXTex images
  nsDynamicArray<Image> outputImages;
  DXGI_FORMAT imageFormat = DXGI_FORMAT(nsImageFormatMappings::ToDxgiFormat(image.GetImageFormat()));
  for (nsUInt32 arrayIdx = 0; arrayIdx < image.GetNumArrayIndices(); ++arrayIdx)
  {
    for (nsUInt32 faceIdx = 0; faceIdx < image.GetNumFaces(); ++faceIdx)
    {
      for (nsUInt32 sliceIdx = 0; sliceIdx < image.GetDepth(); ++sliceIdx)
      {
        Image& currentImage = outputImages.ExpandAndGetRef();
        currentImage.width = image.GetWidth();
        currentImage.height = image.GetHeight();
        currentImage.format = imageFormat;
        currentImage.rowPitch = static_cast<size_t>(image.GetRowPitch());
        currentImage.slicePitch = static_cast<size_t>(image.GetDepthPitch());
        currentImage.pixels = const_cast<uint8_t*>(image.GetPixelPointer<uint8_t>(0, faceIdx, arrayIdx, 0, 0, sliceIdx));
      }
    }
  }

  if (!outputImages.IsEmpty())
  {
    // Store images in output blob
    Blob targetBlob;
    WIC_FLAGS flags = WIC_FLAGS_NONE;
    HRESULT res = SaveToWICMemory(outputImages.GetData(), outputImages.GetCount(), flags, GetWICCodec(WIC_CODEC_TIFF), targetBlob);
    if (FAILED(res))
    {
      nsLog::Error("Failed to save image data to local memory blob - result: {}!", nsHRESULTtoString(res));
      return NS_FAILURE;
    }

    // Push blob into output stream
    if (inout_stream.WriteBytes(targetBlob.GetBufferPointer(), targetBlob.GetBufferSize()) != NS_SUCCESS)
    {
      nsLog::Error("Failed to write image data!");
      return NS_FAILURE;
    }
  }

  return NS_SUCCESS;
}

bool nsWicFileFormat::CanReadFileType(nsStringView sExtension) const
{
  return sExtension.IsEqual_NoCase("png") || sExtension.IsEqual_NoCase("jpg") || sExtension.IsEqual_NoCase("jpeg") ||
         sExtension.IsEqual_NoCase("tif") || sExtension.IsEqual_NoCase("tiff");
}

bool nsWicFileFormat::CanWriteFileType(nsStringView sExtension) const
{
  // png, jpg and jpeg are handled by STB (nsStbImageFileFormats)
  return sExtension.IsEqual_NoCase("tif") || sExtension.IsEqual_NoCase("tiff");
}

#endif



NS_STATICLINK_FILE(Texture, Texture_Image_Formats_WicFileFormat);
