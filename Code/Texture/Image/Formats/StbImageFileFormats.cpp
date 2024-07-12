#include <Texture/TexturePCH.h>

#include <Texture/Image/Formats/StbImageFileFormats.h>
#include <Texture/Image/Image.h>

#include <Foundation/IO/MemoryStream.h>
#include <Foundation/IO/StreamUtils.h>
#include <Foundation/Profiling/Profiling.h>
#include <Texture/Image/ImageConversion.h>
#include <stb_image/stb_image.h>
#include <stb_image/stb_image_write.h>

// NS_STATICLINK_FORCE
nsStbImageFileFormats g_StbImageFormats;

// stb_image callbacks would be better than loading the entire file into memory.
// However, it turned out that it does not map well to nsStreamReader

// namespace
//{
//  // fill 'data' with 'size' bytes.  return number of bytes actually read
//  int read(void *user, char *data, int size)
//  {
//    nsStreamReader* pStream = static_cast<nsStreamReader*>(user);
//    return static_cast<int>(pStream->ReadBytes(data, size));
//  }
//  // skip the next 'n' bytes, or 'unget' the last -n bytes if negative
//  void skip(void *user, int n)
//  {
//    nsStreamReader* pStream = static_cast<nsStreamReader*>(user);
//    if(n > 0)
//      pStream->SkipBytes(n);
//    else
//      // ?? We cannot reverse skip.
//
//  }
//  // returns nonzero if we are at end of file/data
//  int eof(void *user)
//  {
//    nsStreamReader* pStream = static_cast<nsStreamReader*>(user);
//    // ?
//  }
//}

namespace
{

  void write_func(void* pContext, void* pData, int iSize)
  {
    nsStreamWriter* writer = static_cast<nsStreamWriter*>(pContext);
    writer->WriteBytes(pData, iSize).IgnoreResult();
  }

  void* ReadImageData(nsStreamReader& inout_stream, nsDynamicArray<nsUInt8>& ref_fileBuffer, nsImageHeader& ref_imageHeader, bool& ref_bIsHDR)
  {
    nsStreamUtils::ReadAllAndAppend(inout_stream, ref_fileBuffer);

    int width, height, numComp;

    ref_bIsHDR = !!stbi_is_hdr_from_memory(ref_fileBuffer.GetData(), ref_fileBuffer.GetCount());

    void* sourceImageData = nullptr;
    if (ref_bIsHDR)
    {
      sourceImageData = stbi_loadf_from_memory(ref_fileBuffer.GetData(), ref_fileBuffer.GetCount(), &width, &height, &numComp, 0);
    }
    else
    {
      sourceImageData = stbi_load_from_memory(ref_fileBuffer.GetData(), ref_fileBuffer.GetCount(), &width, &height, &numComp, 0);
    }
    if (!sourceImageData)
    {
      nsLog::Error("stb_image failed to load: {0}", stbi_failure_reason());
      return nullptr;
    }
    ref_fileBuffer.Clear();

    nsImageFormat::Enum format = nsImageFormat::UNKNOWN;
    switch (numComp)
    {
      case 1:
        format = (ref_bIsHDR) ? nsImageFormat::R32_FLOAT : nsImageFormat::R8_UNORM;
        break;
      case 2:
        format = (ref_bIsHDR) ? nsImageFormat::R32G32_FLOAT : nsImageFormat::R8G8_UNORM;
        break;
      case 3:
        format = (ref_bIsHDR) ? nsImageFormat::R32G32B32_FLOAT : nsImageFormat::R8G8B8_UNORM;
        break;
      case 4:
        format = (ref_bIsHDR) ? nsImageFormat::R32G32B32A32_FLOAT : nsImageFormat::R8G8B8A8_UNORM;
        break;
    }

    // Set properties and allocate.
    ref_imageHeader.SetImageFormat(format);
    ref_imageHeader.SetNumMipLevels(1);
    ref_imageHeader.SetNumArrayIndices(1);
    ref_imageHeader.SetNumFaces(1);

    ref_imageHeader.SetWidth(width);
    ref_imageHeader.SetHeight(height);
    ref_imageHeader.SetDepth(1);

    return sourceImageData;
  }

} // namespace

nsResult nsStbImageFileFormats::ReadImageHeader(nsStreamReader& inout_stream, nsImageHeader& ref_header, nsStringView sFileExtension) const
{
  NS_PROFILE_SCOPE("nsStbImageFileFormats::ReadImageHeader");

  bool isHDR = false;
  nsDynamicArray<nsUInt8> fileBuffer;
  void* sourceImageData = ReadImageData(inout_stream, fileBuffer, ref_header, isHDR);

  if (sourceImageData == nullptr)
    return NS_FAILURE;

  stbi_image_free(sourceImageData);
  return NS_SUCCESS;
}

nsResult nsStbImageFileFormats::ReadImage(nsStreamReader& inout_stream, nsImage& ref_image, nsStringView sFileExtension) const
{
  NS_PROFILE_SCOPE("nsStbImageFileFormats::ReadImage");

  bool isHDR = false;
  nsDynamicArray<nsUInt8> fileBuffer;
  nsImageHeader imageHeader;
  void* sourceImageData = ReadImageData(inout_stream, fileBuffer, imageHeader, isHDR);

  if (sourceImageData == nullptr)
    return NS_FAILURE;

  ref_image.ResetAndAlloc(imageHeader);

  const size_t numComp = nsImageFormat::GetNumChannels(imageHeader.GetImageFormat());

  const size_t elementsToCopy = static_cast<size_t>(imageHeader.GetWidth()) * static_cast<size_t>(imageHeader.GetHeight()) * numComp;

  // Set pixels. Different strategies depending on component count.
  if (isHDR)
  {
    float* targetImageData = ref_image.GetBlobPtr<float>().GetPtr();
    nsMemoryUtils::Copy(targetImageData, (const float*)sourceImageData, elementsToCopy);
  }
  else
  {
    nsUInt8* targetImageData = ref_image.GetBlobPtr<nsUInt8>().GetPtr();
    nsMemoryUtils::Copy(targetImageData, (const nsUInt8*)sourceImageData, elementsToCopy);
  }

  stbi_image_free((void*)sourceImageData);
  return NS_SUCCESS;
}

nsResult nsStbImageFileFormats::WriteImage(nsStreamWriter& inout_stream, const nsImageView& image, nsStringView sFileExtension) const
{
  nsImageFormat::Enum compatibleFormats[] = {nsImageFormat::R8_UNORM, nsImageFormat::R8G8B8_UNORM, nsImageFormat::R8G8B8A8_UNORM};

  // Find a compatible format closest to the one the image currently has
  nsImageFormat::Enum format = nsImageConversion::FindClosestCompatibleFormat(image.GetImageFormat(), compatibleFormats);

  if (format == nsImageFormat::UNKNOWN)
  {
    nsLog::Error("No conversion from format '{0}' to a format suitable for PNG files known.", nsImageFormat::GetName(image.GetImageFormat()));
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

  if (sFileExtension.IsEqual_NoCase("png"))
  {
    if (stbi_write_png_to_func(write_func, &inout_stream, image.GetWidth(), image.GetHeight(), nsImageFormat::GetNumChannels(image.GetImageFormat()), image.GetByteBlobPtr().GetPtr(), 0))
    {
      return NS_SUCCESS;
    }
  }

  if (sFileExtension.IsEqual_NoCase("jpg") || sFileExtension.IsEqual_NoCase("jpeg"))
  {
    if (stbi_write_jpg_to_func(write_func, &inout_stream, image.GetWidth(), image.GetHeight(), nsImageFormat::GetNumChannels(image.GetImageFormat()), image.GetByteBlobPtr().GetPtr(), 95))
    {
      return NS_SUCCESS;
    }
  }

  return NS_FAILURE;
}

bool nsStbImageFileFormats::CanReadFileType(nsStringView sExtension) const
{
  if (sExtension.IsEqual_NoCase("hdr"))
    return true;

#if NS_DISABLED(NS_PLATFORM_WINDOWS_DESKTOP)

  // on Windows Desktop, we prefer to use WIC (nsWicFileFormat)
  if (sExtension.IsEqual_NoCase("png") || sExtension.IsEqual_NoCase("jpg") || sExtension.IsEqual_NoCase("jpeg"))
  {
    return true;
  }
#endif

  return false;
}

bool nsStbImageFileFormats::CanWriteFileType(nsStringView sExtension) const
{
  // even when WIC is available, prefer to write these files through STB, to get consistent output
  if (sExtension.IsEqual_NoCase("png") || sExtension.IsEqual_NoCase("jpg") || sExtension.IsEqual_NoCase("jpeg"))
  {
    return true;
  }

  return false;
}



NS_STATICLINK_FILE(Texture, Texture_Image_Formats_StbImageFileFormats);
