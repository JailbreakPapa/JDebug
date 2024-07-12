#include <Texture/TexturePCH.h>

#include <Foundation/Basics/Platform/Win/IncludeWindows.h>
#include <Foundation/IO/Stream.h>
#include <Foundation/Profiling/Profiling.h>
#include <Texture/Image/Formats/BmpFileFormat.h>
#include <Texture/Image/ImageConversion.h>

// NS_STATICLINK_FORCE
nsBmpFileFormat g_bmpFormat;

enum nsBmpCompression
{
  RGB = 0L,
  RLE8 = 1L,
  RLE4 = 2L,
  BITFIELDS = 3L,
  JPEG = 4L,
  PNG = 5L,
};


#pragma pack(push, 1)
struct nsBmpFileHeader
{
  nsUInt16 m_type = 0;
  nsUInt32 m_size = 0;
  nsUInt16 m_reserved1 = 0;
  nsUInt16 m_reserved2 = 0;
  nsUInt32 m_offBits = 0;
};
#pragma pack(pop)

struct nsBmpFileInfoHeader
{
  nsUInt32 m_size = 0;
  nsUInt32 m_width = 0;
  nsUInt32 m_height = 0;
  nsUInt16 m_planes = 0;
  nsUInt16 m_bitCount = 0;
  nsBmpCompression m_compression = nsBmpCompression::RGB;
  nsUInt32 m_sizeImage = 0;
  nsUInt32 m_xPelsPerMeter = 0;
  nsUInt32 m_yPelsPerMeter = 0;
  nsUInt32 m_clrUsed = 0;
  nsUInt32 m_clrImportant = 0;
};

struct nsCIEXYZ
{
  int ciexyzX = 0;
  int ciexyzY = 0;
  int ciexyzZ = 0;
};

struct nsCIEXYZTRIPLE
{
  nsCIEXYZ ciexyzRed;
  nsCIEXYZ ciexyzGreen;
  nsCIEXYZ ciexyzBlue;
};

struct nsBmpFileInfoHeaderV4
{
  nsUInt32 m_redMask = 0;
  nsUInt32 m_greenMask = 0;
  nsUInt32 m_blueMask = 0;
  nsUInt32 m_alphaMask = 0;
  nsUInt32 m_csType = 0;
  nsCIEXYZTRIPLE m_endpoints;
  nsUInt32 m_gammaRed = 0;
  nsUInt32 m_gammaGreen = 0;
  nsUInt32 m_gammaBlue = 0;
};

NS_CHECK_AT_COMPILETIME(sizeof(nsCIEXYZTRIPLE) == 3 * 3 * 4);

// just to be on the safe side
#if NS_ENABLED(NS_PLATFORM_WINDOWS)
NS_CHECK_AT_COMPILETIME(sizeof(nsCIEXYZTRIPLE) == sizeof(CIEXYZTRIPLE));
#endif

struct nsBmpFileInfoHeaderV5
{
  nsUInt32 m_intent;
  nsUInt32 m_profileData;
  nsUInt32 m_profileSize;
  nsUInt32 m_reserved;
};

static const nsUInt16 nsBmpFileMagic = 0x4D42u;

struct nsBmpBgrxQuad
{
  NS_DECLARE_POD_TYPE();

  nsBmpBgrxQuad() = default;

  nsBmpBgrxQuad(nsUInt8 uiRed, nsUInt8 uiGreen, nsUInt8 uiBlue)
    : m_blue(uiBlue)
    , m_green(uiGreen)
    , m_red(uiRed)
    , m_reserved(0)
  {
  }

  nsUInt8 m_blue;
  nsUInt8 m_green;
  nsUInt8 m_red;
  nsUInt8 m_reserved;
};

nsResult nsBmpFileFormat::WriteImage(nsStreamWriter& inout_stream, const nsImageView& image, nsStringView sFileExtension) const
{
  // Technically almost arbitrary formats are supported, but we only use the common ones.
  nsImageFormat::Enum compatibleFormats[] = {
    nsImageFormat::B8G8R8X8_UNORM,
    nsImageFormat::B8G8R8A8_UNORM,
    nsImageFormat::B8G8R8_UNORM,
    nsImageFormat::B5G5R5X1_UNORM,
    nsImageFormat::B5G6R5_UNORM,
  };

  // Find a compatible format closest to the one the image currently has
  nsImageFormat::Enum format = nsImageConversion::FindClosestCompatibleFormat(image.GetImageFormat(), compatibleFormats);

  if (format == nsImageFormat::UNKNOWN)
  {
    nsLog::Error("No conversion from format '{0}' to a format suitable for BMP files known.", nsImageFormat::GetName(image.GetImageFormat()));
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

  nsUInt64 uiRowPitch = image.GetRowPitch(0);

  nsUInt32 uiHeight = image.GetHeight(0);

  nsUInt64 dataSize = uiRowPitch * uiHeight;
  if (dataSize >= nsMath::MaxValue<nsUInt32>())
  {
    NS_ASSERT_DEV(false, "Size overflow in BMP file format.");
    return NS_FAILURE;
  }

  nsBmpFileInfoHeader fileInfoHeader;
  fileInfoHeader.m_width = image.GetWidth(0);
  fileInfoHeader.m_height = uiHeight;
  fileInfoHeader.m_planes = 1;
  fileInfoHeader.m_bitCount = static_cast<nsUInt16>(nsImageFormat::GetBitsPerPixel(format));

  fileInfoHeader.m_sizeImage = 0; // Can be zero unless we store the data compressed

  fileInfoHeader.m_xPelsPerMeter = 0;
  fileInfoHeader.m_yPelsPerMeter = 0;
  fileInfoHeader.m_clrUsed = 0;
  fileInfoHeader.m_clrImportant = 0;

  bool bWriteColorMask = false;

  // Prefer to write a V3 header
  nsUInt32 uiHeaderVersion = 3;

  switch (format)
  {
    case nsImageFormat::B8G8R8X8_UNORM:
    case nsImageFormat::B5G5R5X1_UNORM:
    case nsImageFormat::B8G8R8_UNORM:
      fileInfoHeader.m_compression = RGB;
      break;

    case nsImageFormat::B8G8R8A8_UNORM:
      fileInfoHeader.m_compression = BITFIELDS;
      uiHeaderVersion = 4;
      break;

    case nsImageFormat::B5G6R5_UNORM:
      fileInfoHeader.m_compression = BITFIELDS;
      bWriteColorMask = true;
      break;

    default:
      return NS_FAILURE;
  }

  NS_ASSERT_DEV(!bWriteColorMask || uiHeaderVersion <= 3, "Internal bug");

  nsUInt32 uiFileInfoHeaderSize = sizeof(nsBmpFileInfoHeader);
  nsUInt32 uiHeaderSize = sizeof(nsBmpFileHeader);

  if (uiHeaderVersion >= 4)
  {
    uiFileInfoHeaderSize += sizeof(nsBmpFileInfoHeaderV4);
  }
  else if (bWriteColorMask)
  {
    uiHeaderSize += 3 * sizeof(nsUInt32);
  }

  uiHeaderSize += uiFileInfoHeaderSize;

  fileInfoHeader.m_size = uiFileInfoHeaderSize;

  nsBmpFileHeader header;
  header.m_type = nsBmpFileMagic;
  header.m_size = uiHeaderSize + static_cast<nsUInt32>(dataSize);
  header.m_reserved1 = 0;
  header.m_reserved2 = 0;
  header.m_offBits = uiHeaderSize;

  // Write all data
  if (inout_stream.WriteBytes(&header, sizeof(header)) != NS_SUCCESS)
  {
    nsLog::Error("Failed to write header.");
    return NS_FAILURE;
  }

  if (inout_stream.WriteBytes(&fileInfoHeader, sizeof(fileInfoHeader)) != NS_SUCCESS)
  {
    nsLog::Error("Failed to write fileInfoHeader.");
    return NS_FAILURE;
  }

  if (uiHeaderVersion >= 4)
  {
    nsBmpFileInfoHeaderV4 fileInfoHeaderV4;
    memset(&fileInfoHeaderV4, 0, sizeof(fileInfoHeaderV4));

    fileInfoHeaderV4.m_redMask = nsImageFormat::GetRedMask(format);
    fileInfoHeaderV4.m_greenMask = nsImageFormat::GetGreenMask(format);
    fileInfoHeaderV4.m_blueMask = nsImageFormat::GetBlueMask(format);
    fileInfoHeaderV4.m_alphaMask = nsImageFormat::GetAlphaMask(format);

    if (inout_stream.WriteBytes(&fileInfoHeaderV4, sizeof(fileInfoHeaderV4)) != NS_SUCCESS)
    {
      nsLog::Error("Failed to write fileInfoHeaderV4.");
      return NS_FAILURE;
    }
  }
  else if (bWriteColorMask)
  {
    struct
    {
      nsUInt32 m_red;
      nsUInt32 m_green;
      nsUInt32 m_blue;
    } colorMask;


    colorMask.m_red = nsImageFormat::GetRedMask(format);
    colorMask.m_green = nsImageFormat::GetGreenMask(format);
    colorMask.m_blue = nsImageFormat::GetBlueMask(format);

    if (inout_stream.WriteBytes(&colorMask, sizeof(colorMask)) != NS_SUCCESS)
    {
      nsLog::Error("Failed to write colorMask.");
      return NS_FAILURE;
    }
  }

  const nsUInt64 uiPaddedRowPitch = ((uiRowPitch - 1) / 4 + 1) * 4;
  // Write rows in reverse order
  for (nsInt32 iRow = uiHeight - 1; iRow >= 0; iRow--)
  {
    if (inout_stream.WriteBytes(image.GetPixelPointer<void>(0, 0, 0, 0, iRow, 0), uiRowPitch) != NS_SUCCESS)
    {
      nsLog::Error("Failed to write data.");
      return NS_FAILURE;
    }

    nsUInt8 zeroes[4] = {0, 0, 0, 0};
    if (inout_stream.WriteBytes(zeroes, uiPaddedRowPitch - uiRowPitch) != NS_SUCCESS)
    {
      nsLog::Error("Failed to write data.");
      return NS_FAILURE;
    }
  }

  return NS_SUCCESS;
}

namespace
{
  nsUInt32 ExtractBits(const void* pData, nsUInt32 uiBitAddress, nsUInt32 uiNumBits)
  {
    nsUInt32 uiMask = (1U << uiNumBits) - 1;
    nsUInt32 uiByteAddress = uiBitAddress / 8;
    nsUInt32 uiShiftAmount = 7 - (uiBitAddress % 8 + uiNumBits - 1);

    return (reinterpret_cast<const nsUInt8*>(pData)[uiByteAddress] >> uiShiftAmount) & uiMask;
  }

  nsResult ReadImageInfo(nsStreamReader& inout_stream, nsImageHeader& ref_header, nsBmpFileHeader& ref_fileHeader, nsBmpFileInfoHeader& ref_fileInfoHeader, bool& ref_bIndexed,
    bool& ref_bCompressed, nsUInt32& ref_uiBpp, nsUInt32& ref_uiDataSize)
  {
    if (inout_stream.ReadBytes(&ref_fileHeader, sizeof(nsBmpFileHeader)) != sizeof(nsBmpFileHeader))
    {
      nsLog::Error("Failed to read header data.");
      return NS_FAILURE;
    }

    // Some very old BMP variants may have different magic numbers, but we don't support them.
    if (ref_fileHeader.m_type != nsBmpFileMagic)
    {
      nsLog::Error("The file is not a recognized BMP file.");
      return NS_FAILURE;
    }

    // We expect at least header version 3
    nsUInt32 uiHeaderVersion = 3;
    if (inout_stream.ReadBytes(&ref_fileInfoHeader, sizeof(nsBmpFileInfoHeader)) != sizeof(nsBmpFileInfoHeader))
    {
      nsLog::Error("Failed to read header data (V3).");
      return NS_FAILURE;
    }

    int remainingHeaderBytes = ref_fileInfoHeader.m_size - sizeof(ref_fileInfoHeader);

    // File header shorter than expected - happens with corrupt files or e.g. with OS/2 BMP files which may have shorter headers
    if (remainingHeaderBytes < 0)
    {
      nsLog::Error("The file header was shorter than expected.");
      return NS_FAILURE;
    }

    // Newer files may have a header version 4 (required for transparency)
    nsBmpFileInfoHeaderV4 fileInfoHeaderV4;
    if (remainingHeaderBytes >= sizeof(nsBmpFileInfoHeaderV4))
    {
      uiHeaderVersion = 4;
      if (inout_stream.ReadBytes(&fileInfoHeaderV4, sizeof(nsBmpFileInfoHeaderV4)) != sizeof(nsBmpFileInfoHeaderV4))
      {
        nsLog::Error("Failed to read header data (V4).");
        return NS_FAILURE;
      }
      remainingHeaderBytes -= sizeof(nsBmpFileInfoHeaderV4);
    }

    // Skip rest of header
    if (inout_stream.SkipBytes(remainingHeaderBytes) != remainingHeaderBytes)
    {
      nsLog::Error("Failed to skip remaining header data.");
      return NS_FAILURE;
    }

    ref_uiBpp = ref_fileInfoHeader.m_bitCount;

    // Find target format to load the image
    nsImageFormat::Enum format = nsImageFormat::UNKNOWN;

    switch (ref_fileInfoHeader.m_compression)
    {
        // RGB or indexed data
      case RGB:
        switch (ref_uiBpp)
        {
          case 1:
          case 4:
          case 8:
            ref_bIndexed = true;

            // We always decompress indexed to BGRX, since the palette is specified in this format
            format = nsImageFormat::B8G8R8X8_UNORM;
            break;

          case 16:
            format = nsImageFormat::B5G5R5X1_UNORM;
            break;

          case 24:
            format = nsImageFormat::B8G8R8_UNORM;
            break;

          case 32:
            format = nsImageFormat::B8G8R8X8_UNORM;
        }
        break;

        // RGB data, but with the color masks specified in place of the palette
      case BITFIELDS:
        switch (ref_uiBpp)
        {
          case 16:
          case 32:
            // In case of old headers, the color masks appear after the header (and aren't counted as part of it)
            if (uiHeaderVersion < 4)
            {
              // Color masks (w/o alpha channel)
              struct
              {
                nsUInt32 m_red;
                nsUInt32 m_green;
                nsUInt32 m_blue;
              } colorMask;

              if (inout_stream.ReadBytes(&colorMask, sizeof(colorMask)) != sizeof(colorMask))
              {
                return NS_FAILURE;
              }

              format = nsImageFormat::FromPixelMask(colorMask.m_red, colorMask.m_green, colorMask.m_blue, 0, ref_uiBpp);
            }
            else
            {
              // For header version four and higher, the color masks are part of the header
              format = nsImageFormat::FromPixelMask(
                fileInfoHeaderV4.m_redMask, fileInfoHeaderV4.m_greenMask, fileInfoHeaderV4.m_blueMask, fileInfoHeaderV4.m_alphaMask, ref_uiBpp);
            }

            break;
        }
        break;

      case RLE4:
        if (ref_uiBpp == 4)
        {
          ref_bIndexed = true;
          ref_bCompressed = true;
          format = nsImageFormat::B8G8R8X8_UNORM;
        }
        break;

      case RLE8:
        if (ref_uiBpp == 8)
        {
          ref_bIndexed = true;
          ref_bCompressed = true;
          format = nsImageFormat::B8G8R8X8_UNORM;
        }
        break;

      default:
        NS_ASSERT_NOT_IMPLEMENTED;
    }

    if (format == nsImageFormat::UNKNOWN)
    {
      nsLog::Error("Unknown or unsupported BMP encoding.");
      return NS_FAILURE;
    }

    const nsUInt32 uiWidth = ref_fileInfoHeader.m_width;

    if (uiWidth > 65536)
    {
      nsLog::Error("Image specifies width > 65536. Header corrupted?");
      return NS_FAILURE;
    }

    const nsUInt32 uiHeight = ref_fileInfoHeader.m_height;

    if (uiHeight > 65536)
    {
      nsLog::Error("Image specifies height > 65536. Header corrupted?");
      return NS_FAILURE;
    }

    ref_uiDataSize = ref_fileInfoHeader.m_sizeImage;

    if (ref_uiDataSize > 1024 * 1024 * 1024)
    {
      nsLog::Error("Image specifies data size > 1GiB. Header corrupted?");
      return NS_FAILURE;
    }

    const int uiRowPitchIn = (uiWidth * ref_uiBpp + 31) / 32 * 4;

    if (ref_uiDataSize == 0)
    {
      if (ref_fileInfoHeader.m_compression != RGB)
      {
        nsLog::Error("The data size wasn't specified in the header.");
        return NS_FAILURE;
      }
      ref_uiDataSize = uiRowPitchIn * uiHeight;
    }

    // Set image data
    ref_header.SetImageFormat(format);
    ref_header.SetNumMipLevels(1);
    ref_header.SetNumArrayIndices(1);
    ref_header.SetNumFaces(1);

    ref_header.SetWidth(uiWidth);
    ref_header.SetHeight(uiHeight);
    ref_header.SetDepth(1);

    return NS_SUCCESS;
  }

} // namespace

nsResult nsBmpFileFormat::ReadImageHeader(nsStreamReader& inout_stream, nsImageHeader& ref_header, nsStringView sFileExtension) const
{
  NS_PROFILE_SCOPE("nsBmpFileFormat::ReadImage");

  nsBmpFileHeader fileHeader;
  nsBmpFileInfoHeader fileInfoHeader;
  bool bIndexed = false, bCompressed = false;
  nsUInt32 uiBpp = 0;
  nsUInt32 uiDataSize = 0;

  return ReadImageInfo(inout_stream, ref_header, fileHeader, fileInfoHeader, bIndexed, bCompressed, uiBpp, uiDataSize);
}

nsResult nsBmpFileFormat::ReadImage(nsStreamReader& inout_stream, nsImage& ref_image, nsStringView sFileExtension) const
{
  NS_PROFILE_SCOPE("nsBmpFileFormat::ReadImage");

  nsBmpFileHeader fileHeader;
  nsImageHeader header;
  nsBmpFileInfoHeader fileInfoHeader;
  bool bIndexed = false, bCompressed = false;
  nsUInt32 uiBpp = 0;
  nsUInt32 uiDataSize = 0;

  NS_SUCCEED_OR_RETURN(ReadImageInfo(inout_stream, header, fileHeader, fileInfoHeader, bIndexed, bCompressed, uiBpp, uiDataSize));

  ref_image.ResetAndAlloc(header);

  nsUInt64 uiRowPitch = ref_image.GetRowPitch(0);

  const int uiRowPitchIn = (header.GetWidth() * uiBpp + 31) / 32 * 4;

  if (bIndexed)
  {
    // If no palette size was specified, the full available palette size will be used
    nsUInt32 paletteSize = fileInfoHeader.m_clrUsed;
    if (paletteSize == 0)
    {
      paletteSize = 1U << uiBpp;
    }
    else if (paletteSize > 65536)
    {
      nsLog::Error("Palette size > 65536.");
      return NS_FAILURE;
    }

    nsDynamicArray<nsBmpBgrxQuad> palette;
    palette.SetCountUninitialized(paletteSize);
    if (inout_stream.ReadBytes(&palette[0], paletteSize * sizeof(nsBmpBgrxQuad)) != paletteSize * sizeof(nsBmpBgrxQuad))
    {
      nsLog::Error("Failed to read palette data.");
      return NS_FAILURE;
    }

    if (bCompressed)
    {
      // Compressed data is always in pairs of bytes
      if (uiDataSize % 2 != 0)
      {
        nsLog::Error("The data size is not a multiple of 2 bytes in an RLE-compressed file.");
        return NS_FAILURE;
      }

      nsDynamicArray<nsUInt8> compressedData;
      compressedData.SetCountUninitialized(uiDataSize);

      if (inout_stream.ReadBytes(&compressedData[0], uiDataSize) != uiDataSize)
      {
        nsLog::Error("Failed to read data.");
        return NS_FAILURE;
      }

      const nsUInt8* pIn = &compressedData[0];
      const nsUInt8* pInEnd = pIn + uiDataSize;

      // Current output position
      nsUInt32 uiRow = fileInfoHeader.m_height - 1;
      nsUInt32 uiCol = 0;

      nsBmpBgrxQuad* pLine = ref_image.GetPixelPointer<nsBmpBgrxQuad>(0, 0, 0, 0, uiRow, 0);

      // Decode RLE data directly to RGBX
      while (pIn < pInEnd)
      {
        nsUInt32 uiByte1 = *pIn++;
        nsUInt32 uiByte2 = *pIn++;

        // Relative mode - the first byte specified a number of indices to be repeated, the second one the indices
        if (uiByte1 > 0)
        {
          // Clamp number of repetitions to row width.
          // The spec isn't clear on this point, but some files pad the number of encoded indices for some reason.
          uiByte1 = nsMath::Min(uiByte1, fileInfoHeader.m_width - uiCol);

          if (uiBpp == 4)
          {
            // Alternate between two indices.
            for (nsUInt32 uiRep = 0; uiRep < uiByte1 / 2; uiRep++)
            {
              pLine[uiCol++] = palette[uiByte2 >> 4];
              pLine[uiCol++] = palette[uiByte2 & 0x0F];
            }

            // Repeat the first index for odd numbers of repetitions.
            if (uiByte1 & 1)
            {
              pLine[uiCol++] = palette[uiByte2 >> 4];
            }
          }
          else /* if (uiBpp == 8) */
          {
            // Repeat a single index.
            for (nsUInt32 uiRep = 0; uiRep < uiByte1; uiRep++)
            {
              pLine[uiCol++] = palette[uiByte2];
            }
          }
        }
        else
        {
          // Absolute mode - the first byte specifies a number of indices encoded separately, or is a special marker
          switch (uiByte2)
          {
              // End of line marker
            case 0:
            {

              // Fill up with palette entry 0
              while (uiCol < fileInfoHeader.m_width)
              {
                pLine[uiCol++] = palette[0];
              }

              // Begin next line
              uiCol = 0;
              uiRow--;
              pLine -= fileInfoHeader.m_width;
            }

            break;

              // End of image marker
            case 1:
              // Check that we really reached the end of the image.
              if (uiRow != 0 && uiCol != fileInfoHeader.m_height - 1)
              {
                nsLog::Error("Unexpected end of image marker found.");
                return NS_FAILURE;
              }
              break;

            case 2:
              nsLog::Error("Found a RLE compression position delta - this is not supported.");
              return NS_FAILURE;

            default:
              // Read uiByte2 number of indices

              // More data than fits into the image or can be read?
              if (uiCol + uiByte2 > fileInfoHeader.m_width || pIn + (uiByte2 + 1) / 2 > pInEnd)
              {
                return NS_FAILURE;
              }

              if (uiBpp == 4)
              {
                for (nsUInt32 uiRep = 0; uiRep < uiByte2 / 2; uiRep++)
                {
                  nsUInt32 uiIndices = *pIn++;
                  pLine[uiCol++] = palette[uiIndices >> 4];
                  pLine[uiCol++] = palette[uiIndices & 0x0f];
                }

                if (uiByte2 & 1)
                {
                  pLine[uiCol++] = palette[*pIn++ >> 4];
                }

                // Pad to word boundary
                pIn += (uiByte2 / 2 + uiByte2) & 1;
              }
              else /* if (uiBpp == 8) */
              {
                for (nsUInt32 uiRep = 0; uiRep < uiByte2; uiRep++)
                {
                  pLine[uiCol++] = palette[*pIn++];
                }

                // Pad to word boundary
                pIn += uiByte2 & 1;
              }
          }
        }
      }
    }
    else
    {
      nsDynamicArray<nsUInt8> indexedData;
      indexedData.SetCountUninitialized(uiDataSize);
      if (inout_stream.ReadBytes(&indexedData[0], uiDataSize) != uiDataSize)
      {
        nsLog::Error("Failed to read data.");
        return NS_FAILURE;
      }

      // Convert to non-indexed
      for (nsUInt32 uiRow = 0; uiRow < fileInfoHeader.m_height; uiRow++)
      {
        nsUInt8* pIn = &indexedData[uiRowPitchIn * uiRow];

        // Convert flipped vertically
        nsBmpBgrxQuad* pOut = ref_image.GetPixelPointer<nsBmpBgrxQuad>(0, 0, 0, 0, fileInfoHeader.m_height - uiRow - 1, 0);
        for (nsUInt32 uiCol = 0; uiCol < ref_image.GetWidth(0); uiCol++)
        {
          nsUInt32 uiIndex = ExtractBits(pIn, uiCol * uiBpp, uiBpp);
          if (uiIndex >= palette.GetCount())
          {
            nsLog::Error("Image contains invalid palette indices.");
            return NS_FAILURE;
          }
          pOut[uiCol] = palette[uiIndex];
        }
      }
    }
  }
  else
  {
    // Format must match the number of bits in the file
    if (nsImageFormat::GetBitsPerPixel(header.GetImageFormat()) != uiBpp)
    {
      nsLog::Error("The number of bits per pixel specified in the file ({0}) does not match the expected value of {1} for the format '{2}'.",
        uiBpp, nsImageFormat::GetBitsPerPixel(header.GetImageFormat()), nsImageFormat::GetName(header.GetImageFormat()));
      return NS_FAILURE;
    }

    // Skip palette data. Having a palette here doesn't make sense, but is not explicitly disallowed by the standard.
    nsUInt32 paletteSize = fileInfoHeader.m_clrUsed * sizeof(nsBmpBgrxQuad);
    if (inout_stream.SkipBytes(paletteSize) != paletteSize)
    {
      nsLog::Error("Failed to skip palette data.");
      return NS_FAILURE;
    }

    // Read rows in reverse order
    for (nsInt32 iRow = fileInfoHeader.m_height - 1; iRow >= 0; iRow--)
    {
      if (inout_stream.ReadBytes(ref_image.GetPixelPointer<void>(0, 0, 0, 0, iRow, 0), uiRowPitch) != uiRowPitch)
      {
        nsLog::Error("Failed to read row data.");
        return NS_FAILURE;
      }
      if (inout_stream.SkipBytes(uiRowPitchIn - uiRowPitch) != uiRowPitchIn - uiRowPitch)
      {
        nsLog::Error("Failed to skip row data.");
        return NS_FAILURE;
      }
    }
  }

  return NS_SUCCESS;
}

bool nsBmpFileFormat::CanReadFileType(nsStringView sExtension) const
{
  return sExtension.IsEqual_NoCase("bmp") || sExtension.IsEqual_NoCase("dib") || sExtension.IsEqual_NoCase("rle");
}

bool nsBmpFileFormat::CanWriteFileType(nsStringView sExtension) const
{
  return CanReadFileType(sExtension);
}



NS_STATICLINK_FILE(Texture, Texture_Image_Formats_BmpFileFormat);
