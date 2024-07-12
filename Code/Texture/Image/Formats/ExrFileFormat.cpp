#include <Texture/TexturePCH.h>

#if NS_DISABLED(NS_PLATFORM_WINDOWS_UWP)

#  include <Texture/Image/Formats/ExrFileFormat.h>
#  include <Texture/Image/Image.h>

#  include <Foundation/IO/MemoryStream.h>
#  include <Foundation/IO/StreamUtils.h>
#  include <Foundation/Profiling/Profiling.h>

#  include <tinyexr/tinyexr.h>

// NS_STATICLINK_FORCE
nsExrFileFormat g_ExrFileFormat;

nsResult ReadImageData(nsStreamReader& ref_stream, nsDynamicArray<nsUInt8>& ref_fileBuffer, nsImageHeader& ref_header, EXRHeader& ref_exrHeader, EXRImage& ref_exrImage)
{
  // read the entire file to memory
  nsStreamUtils::ReadAllAndAppend(ref_stream, ref_fileBuffer);

  // read the EXR version
  EXRVersion exrVersion;

  if (ParseEXRVersionFromMemory(&exrVersion, ref_fileBuffer.GetData(), ref_fileBuffer.GetCount()) != 0)
  {
    nsLog::Error("Invalid EXR file: Cannot read version.");
    return NS_FAILURE;
  }

  if (exrVersion.multipart)
  {
    nsLog::Error("Invalid EXR file: Multi-part formats are not supported.");
    return NS_FAILURE;
  }

  // read the EXR header
  const char* err = nullptr;
  if (ParseEXRHeaderFromMemory(&ref_exrHeader, &exrVersion, ref_fileBuffer.GetData(), ref_fileBuffer.GetCount(), &err) != 0)
  {
    nsLog::Error("Invalid EXR file: '{0}'", err);
    FreeEXRErrorMessage(err);
    return NS_FAILURE;
  }

  for (int c = 1; c < ref_exrHeader.num_channels; ++c)
  {
    if (ref_exrHeader.pixel_types[c - 1] != ref_exrHeader.pixel_types[c])
    {
      nsLog::Error("Unsupported EXR file: all channels should have the same size.");
      break;
    }
  }

  if (LoadEXRImageFromMemory(&ref_exrImage, &ref_exrHeader, ref_fileBuffer.GetData(), ref_fileBuffer.GetCount(), &err) != 0)
  {
    nsLog::Error("Invalid EXR file: '{0}'", err);

    FreeEXRHeader(&ref_exrHeader);
    FreeEXRErrorMessage(err);
    return NS_FAILURE;
  }

  nsImageFormat::Enum imageFormat = nsImageFormat::UNKNOWN;

  switch (ref_exrHeader.num_channels)
  {
    case 1:
    {
      switch (ref_exrHeader.pixel_types[0])
      {
        case TINYEXR_PIXELTYPE_FLOAT:
          imageFormat = nsImageFormat::R32_FLOAT;
          break;

        case TINYEXR_PIXELTYPE_HALF:
          imageFormat = nsImageFormat::R16_FLOAT;
          break;

        case TINYEXR_PIXELTYPE_UINT:
          imageFormat = nsImageFormat::R32_UINT;
          break;
      }

      break;
    }

    case 2:
    {
      switch (ref_exrHeader.pixel_types[0])
      {
        case TINYEXR_PIXELTYPE_FLOAT:
          imageFormat = nsImageFormat::R32G32_FLOAT;
          break;

        case TINYEXR_PIXELTYPE_HALF:
          imageFormat = nsImageFormat::R16G16_FLOAT;
          break;

        case TINYEXR_PIXELTYPE_UINT:
          imageFormat = nsImageFormat::R32G32_UINT;
          break;
      }

      break;
    }

    case 3:
    {
      switch (ref_exrHeader.pixel_types[0])
      {
        case TINYEXR_PIXELTYPE_FLOAT:
          imageFormat = nsImageFormat::R32G32B32_FLOAT;
          break;

        case TINYEXR_PIXELTYPE_HALF:
          imageFormat = nsImageFormat::R16G16B16A16_FLOAT;
          break;

        case TINYEXR_PIXELTYPE_UINT:
          imageFormat = nsImageFormat::R32G32B32_UINT;
          break;
      }

      break;
    }

    case 4:
    {
      switch (ref_exrHeader.pixel_types[0])
      {
        case TINYEXR_PIXELTYPE_FLOAT:
          imageFormat = nsImageFormat::R32G32B32A32_FLOAT;
          break;

        case TINYEXR_PIXELTYPE_HALF:
          imageFormat = nsImageFormat::R16G16B16A16_FLOAT;
          break;

        case TINYEXR_PIXELTYPE_UINT:
          imageFormat = nsImageFormat::R32G32B32A32_UINT;
          break;
      }

      break;
    }
  }

  if (imageFormat == nsImageFormat::UNKNOWN)
  {
    nsLog::Error("Unsupported EXR file: {}-channel files with format '{}' are unsupported.", ref_exrHeader.num_channels, ref_exrHeader.pixel_types[0]);
    return NS_FAILURE;
  }

  ref_header.SetWidth(ref_exrImage.width);
  ref_header.SetHeight(ref_exrImage.height);
  ref_header.SetImageFormat(imageFormat);

  ref_header.SetNumMipLevels(1);
  ref_header.SetNumArrayIndices(1);
  ref_header.SetNumFaces(1);
  ref_header.SetDepth(1);

  return NS_SUCCESS;
}

nsResult nsExrFileFormat::ReadImageHeader(nsStreamReader& ref_stream, nsImageHeader& ref_header, nsStringView sFileExtension) const
{
  NS_PROFILE_SCOPE("nsExrFileFormat::ReadImageHeader");

  EXRHeader exrHeader;
  InitEXRHeader(&exrHeader);
  NS_SCOPE_EXIT(FreeEXRHeader(&exrHeader));

  EXRImage exrImage;
  InitEXRImage(&exrImage);
  NS_SCOPE_EXIT(FreeEXRImage(&exrImage));

  nsDynamicArray<nsUInt8> fileBuffer;
  return ReadImageData(ref_stream, fileBuffer, ref_header, exrHeader, exrImage);
}

static void CopyChannel(nsUInt8* pDst, const nsUInt8* pSrc, nsUInt32 uiNumElements, nsUInt32 uiElementSize, nsUInt32 uiDstStride)
{
  if (uiDstStride == uiElementSize)
  {
    // fast path to copy everything in one operation
    // this only happens for single-channel formats
    nsMemoryUtils::RawByteCopy(pDst, pSrc, uiNumElements * uiElementSize);
  }
  else
  {
    for (nsUInt32 i = 0; i < uiNumElements; ++i)
    {
      nsMemoryUtils::RawByteCopy(pDst, pSrc, uiElementSize);

      pSrc = nsMemoryUtils::AddByteOffset(pSrc, uiElementSize);
      pDst = nsMemoryUtils::AddByteOffset(pDst, uiDstStride);
    }
  }
}

nsResult nsExrFileFormat::ReadImage(nsStreamReader& ref_stream, nsImage& ref_image, nsStringView sFileExtension) const
{
  NS_PROFILE_SCOPE("nsExrFileFormat::ReadImage");

  EXRHeader exrHeader;
  InitEXRHeader(&exrHeader);
  NS_SCOPE_EXIT(FreeEXRHeader(&exrHeader));

  EXRImage exrImage;
  InitEXRImage(&exrImage);
  NS_SCOPE_EXIT(FreeEXRImage(&exrImage));

  nsImageHeader header;
  nsDynamicArray<nsUInt8> fileBuffer;

  NS_SUCCEED_OR_RETURN(ReadImageData(ref_stream, fileBuffer, header, exrHeader, exrImage));

  ref_image.ResetAndAlloc(header);

  const nsUInt32 uiPixelCount = header.GetWidth() * header.GetHeight();
  const nsUInt32 uiNumDstChannels = nsImageFormat::GetNumChannels(header.GetImageFormat());
  const nsUInt32 uiNumSrcChannels = exrHeader.num_channels;

  nsUInt32 uiSrcStride = 0;
  switch (exrHeader.pixel_types[0])
  {
    case TINYEXR_PIXELTYPE_FLOAT:
      uiSrcStride = sizeof(float);
      break;

    case TINYEXR_PIXELTYPE_HALF:
      uiSrcStride = sizeof(float) / 2;
      break;

    case TINYEXR_PIXELTYPE_UINT:
      uiSrcStride = sizeof(nsUInt32);
      break;

      NS_DEFAULT_CASE_NOT_IMPLEMENTED;
  }


  // src and dst element size is always identical, we only copy from float->float, half->half or uint->uint
  // however data is interleaved in dst, but not interleaved in src

  const nsUInt32 uiDstStride = uiSrcStride * uiNumDstChannels;
  nsUInt8* pDstBytes = ref_image.GetBlobPtr<nsUInt8>().GetPtr();

  if (uiNumDstChannels > uiNumSrcChannels)
  {
    // if we have more dst channels, than in the input data, fill everything with white
    nsMemoryUtils::PatternFill(pDstBytes, 0xFF, uiDstStride * uiPixelCount);
  }

  nsUInt32 c = 0;

  if (uiNumSrcChannels >= 4)
  {
    const nsUInt8* pSrcBytes = exrImage.images[c++];

    if (uiNumDstChannels >= 4)
    {
      // copy to alpha
      CopyChannel(pDstBytes + 3 * uiSrcStride, pSrcBytes, uiPixelCount, uiSrcStride, uiDstStride);
    }
  }

  if (uiNumSrcChannels >= 3)
  {
    const nsUInt8* pSrcBytes = exrImage.images[c++];

    if (uiNumDstChannels >= 3)
    {
      // copy to blue
      CopyChannel(pDstBytes + 2 * uiSrcStride, pSrcBytes, uiPixelCount, uiSrcStride, uiDstStride);
    }
  }

  if (uiNumSrcChannels >= 2)
  {
    const nsUInt8* pSrcBytes = exrImage.images[c++];

    if (uiNumDstChannels >= 2)
    {
      // copy to green
      CopyChannel(pDstBytes + uiSrcStride, pSrcBytes, uiPixelCount, uiSrcStride, uiDstStride);
    }
  }

  if (uiNumSrcChannels >= 1)
  {
    const nsUInt8* pSrcBytes = exrImage.images[c++];

    if (uiNumDstChannels >= 1)
    {
      // copy to red
      CopyChannel(pDstBytes, pSrcBytes, uiPixelCount, uiSrcStride, uiDstStride);
    }
  }

  return NS_SUCCESS;
}

nsResult nsExrFileFormat::WriteImage(nsStreamWriter& ref_stream, const nsImageView& image, nsStringView sFileExtension) const
{
  NS_ASSERT_NOT_IMPLEMENTED;
  return NS_FAILURE;
}

bool nsExrFileFormat::CanReadFileType(nsStringView sExtension) const
{
  return sExtension.IsEqual_NoCase("exr");
}

bool nsExrFileFormat::CanWriteFileType(nsStringView sExtension) const
{
  return false;
}

#endif



NS_STATICLINK_FILE(Texture, Texture_Image_Formats_ExrFileFormat);
