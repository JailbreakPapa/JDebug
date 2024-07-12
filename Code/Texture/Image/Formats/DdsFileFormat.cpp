#include <Texture/TexturePCH.h>

#include <Foundation/IO/Stream.h>
#include <Foundation/Profiling/Profiling.h>
#include <Texture/Image/Formats/DdsFileFormat.h>
#include <Texture/Image/Formats/ImageFormatMappings.h>
#include <Texture/Image/Image.h>

// NS_STATICLINK_FORCE
nsDdsFileFormat g_ddsFormat;

struct nsDdsPixelFormat
{
  nsUInt32 m_uiSize;
  nsUInt32 m_uiFlags;
  nsUInt32 m_uiFourCC;
  nsUInt32 m_uiRGBBitCount;
  nsUInt32 m_uiRBitMask;
  nsUInt32 m_uiGBitMask;
  nsUInt32 m_uiBBitMask;
  nsUInt32 m_uiABitMask;
};

struct nsDdsHeader
{
  nsUInt32 m_uiMagic;
  nsUInt32 m_uiSize;
  nsUInt32 m_uiFlags;
  nsUInt32 m_uiHeight;
  nsUInt32 m_uiWidth;
  nsUInt32 m_uiPitchOrLinearSize;
  nsUInt32 m_uiDepth;
  nsUInt32 m_uiMipMapCount;
  nsUInt32 m_uiReserved1[11];
  nsDdsPixelFormat m_ddspf;
  nsUInt32 m_uiCaps;
  nsUInt32 m_uiCaps2;
  nsUInt32 m_uiCaps3;
  nsUInt32 m_uiCaps4;
  nsUInt32 m_uiReserved2;
};

struct nsDdsResourceDimension
{
  enum Enum
  {
    TEXTURE1D = 2,
    TEXTURE2D = 3,
    TEXTURE3D = 4,
  };
};

struct nsDdsResourceMiscFlags
{
  enum Enum
  {
    TEXTURECUBE = 0x4,
  };
};

struct nsDdsHeaderDxt10
{
  nsUInt32 m_uiDxgiFormat;
  nsUInt32 m_uiResourceDimension;
  nsUInt32 m_uiMiscFlag;
  nsUInt32 m_uiArraySize;
  nsUInt32 m_uiMiscFlags2;
};

struct nsDdsdFlags
{
  enum Enum
  {
    CAPS = 0x000001,
    HEIGHT = 0x000002,
    WIDTH = 0x000004,
    PITCH = 0x000008,
    PIXELFORMAT = 0x001000,
    MIPMAPCOUNT = 0x020000,
    LINEARSIZE = 0x080000,
    DEPTH = 0x800000,
  };
};

struct nsDdpfFlags
{
  enum Enum
  {
    ALPHAPIXELS = 0x00001,
    ALPHA = 0x00002,
    FOURCC = 0x00004,
    RGB = 0x00040,
    YUV = 0x00200,
    LUMINANCE = 0x20000,
  };
};

struct nsDdsCaps
{
  enum Enum
  {
    COMPLEX = 0x000008,
    MIPMAP = 0x400000,
    TEXTURE = 0x001000,
  };
};

struct nsDdsCaps2
{
  enum Enum
  {
    CUBEMAP = 0x000200,
    CUBEMAP_POSITIVEX = 0x000400,
    CUBEMAP_NEGATIVEX = 0x000800,
    CUBEMAP_POSITIVEY = 0x001000,
    CUBEMAP_NEGATIVEY = 0x002000,
    CUBEMAP_POSITIVNS = 0x004000,
    CUBEMAP_NEGATIVNS = 0x008000,
    VOLUME = 0x200000,
  };
};

static const nsUInt32 nsDdsMagic = 0x20534444;
static const nsUInt32 nsDdsDxt10FourCc = 0x30315844;

static nsResult ReadImageData(nsStreamReader& inout_stream, nsImageHeader& ref_imageHeader, nsDdsHeader& ref_ddsHeader)
{
  if (inout_stream.ReadBytes(&ref_ddsHeader, sizeof(nsDdsHeader)) != sizeof(nsDdsHeader))
  {
    nsLog::Error("Failed to read file header.");
    return NS_FAILURE;
  }

  if (ref_ddsHeader.m_uiMagic != nsDdsMagic)
  {
    nsLog::Error("The file is not a recognized DDS file.");
    return NS_FAILURE;
  }

  if (ref_ddsHeader.m_uiSize != 124)
  {
    nsLog::Error("The file header size {0} doesn't match the expected size of 124.", ref_ddsHeader.m_uiSize);
    return NS_FAILURE;
  }

  // Required in every .dds file. According to the spec, CAPS and PIXELFORMAT are also required, but D3DX outputs
  // files not conforming to this.
  if ((ref_ddsHeader.m_uiFlags & nsDdsdFlags::WIDTH) == 0 || (ref_ddsHeader.m_uiFlags & nsDdsdFlags::HEIGHT) == 0)
  {
    nsLog::Error("The file header doesn't specify the mandatory WIDTH or HEIGHT flag.");
    return NS_FAILURE;
  }

  if ((ref_ddsHeader.m_uiCaps & nsDdsCaps::TEXTURE) == 0)
  {
    nsLog::Error("The file header doesn't specify the mandatory TEXTURE flag.");
    return NS_FAILURE;
  }

  ref_imageHeader.SetWidth(ref_ddsHeader.m_uiWidth);
  ref_imageHeader.SetHeight(ref_ddsHeader.m_uiHeight);

  if (ref_ddsHeader.m_ddspf.m_uiSize != 32)
  {
    nsLog::Error("The pixel format size {0} doesn't match the expected value of 32.", ref_ddsHeader.m_ddspf.m_uiSize);
    return NS_FAILURE;
  }

  nsDdsHeaderDxt10 headerDxt10;

  nsImageFormat::Enum format = nsImageFormat::UNKNOWN;

  // Data format specified in RGBA masks
  if ((ref_ddsHeader.m_ddspf.m_uiFlags & nsDdpfFlags::ALPHAPIXELS) != 0 || (ref_ddsHeader.m_ddspf.m_uiFlags & nsDdpfFlags::RGB) != 0 ||
      (ref_ddsHeader.m_ddspf.m_uiFlags & nsDdpfFlags::ALPHA) != 0)
  {
    format = nsImageFormat::FromPixelMask(ref_ddsHeader.m_ddspf.m_uiRBitMask, ref_ddsHeader.m_ddspf.m_uiGBitMask, ref_ddsHeader.m_ddspf.m_uiBBitMask,
      ref_ddsHeader.m_ddspf.m_uiABitMask, ref_ddsHeader.m_ddspf.m_uiRGBBitCount);

    if (format == nsImageFormat::UNKNOWN)
    {
      nsLog::Error("The pixel mask specified was not recognized (R: {0}, G: {1}, B: {2}, A: {3}, Bpp: {4}).",
        nsArgU(ref_ddsHeader.m_ddspf.m_uiRBitMask, 1, false, 16), nsArgU(ref_ddsHeader.m_ddspf.m_uiGBitMask, 1, false, 16),
        nsArgU(ref_ddsHeader.m_ddspf.m_uiBBitMask, 1, false, 16), nsArgU(ref_ddsHeader.m_ddspf.m_uiABitMask, 1, false, 16),
        ref_ddsHeader.m_ddspf.m_uiRGBBitCount);
      return NS_FAILURE;
    }

    // Verify that the format we found is correct
    if (nsImageFormat::GetBitsPerPixel(format) != ref_ddsHeader.m_ddspf.m_uiRGBBitCount)
    {
      nsLog::Error("The number of bits per pixel specified in the file ({0}) does not match the expected value of {1} for the format '{2}'.",
        ref_ddsHeader.m_ddspf.m_uiRGBBitCount, nsImageFormat::GetBitsPerPixel(format), nsImageFormat::GetName(format));
      return NS_FAILURE;
    }
  }
  else if ((ref_ddsHeader.m_ddspf.m_uiFlags & nsDdpfFlags::FOURCC) != 0)
  {
    if (ref_ddsHeader.m_ddspf.m_uiFourCC == nsDdsDxt10FourCc)
    {
      if (inout_stream.ReadBytes(&headerDxt10, sizeof(nsDdsHeaderDxt10)) != sizeof(nsDdsHeaderDxt10))
      {
        nsLog::Error("Failed to read file header.");
        return NS_FAILURE;
      }

      format = nsImageFormatMappings::FromDxgiFormat(headerDxt10.m_uiDxgiFormat);

      if (format == nsImageFormat::UNKNOWN)
      {
        nsLog::Error("The DXGI format {0} has no equivalent image format.", headerDxt10.m_uiDxgiFormat);
        return NS_FAILURE;
      }
    }
    else
    {
      format = nsImageFormatMappings::FromFourCc(ref_ddsHeader.m_ddspf.m_uiFourCC);

      if (format == nsImageFormat::UNKNOWN)
      {
        nsLog::Error("The FourCC code '{0}{1}{2}{3}' was not recognized.", nsArgC((char)(ref_ddsHeader.m_ddspf.m_uiFourCC >> 0)),
          nsArgC((char)(ref_ddsHeader.m_ddspf.m_uiFourCC >> 8)), nsArgC((char)(ref_ddsHeader.m_ddspf.m_uiFourCC >> 16)),
          nsArgC((char)(ref_ddsHeader.m_ddspf.m_uiFourCC >> 24)));
        return NS_FAILURE;
      }
    }
  }
  else
  {
    nsLog::Error("The image format is neither specified as a pixel mask nor as a FourCC code.");
    return NS_FAILURE;
  }

  ref_imageHeader.SetImageFormat(format);

  const bool bHasMipMaps = (ref_ddsHeader.m_uiCaps & nsDdsCaps::MIPMAP) != 0;
  const bool bCubeMap = (ref_ddsHeader.m_uiCaps2 & nsDdsCaps2::CUBEMAP) != 0;
  const bool bVolume = (ref_ddsHeader.m_uiCaps2 & nsDdsCaps2::VOLUME) != 0;


  if (bHasMipMaps)
  {
    ref_imageHeader.SetNumMipLevels(ref_ddsHeader.m_uiMipMapCount);
  }

  // Cubemap and volume texture are mutually exclusive
  if (bVolume && bCubeMap)
  {
    nsLog::Error("The header specifies both the VOLUME and CUBEMAP flags.");
    return NS_FAILURE;
  }

  if (bCubeMap)
  {
    ref_imageHeader.SetNumFaces(6);
  }
  else if (bVolume)
  {
    ref_imageHeader.SetDepth(ref_ddsHeader.m_uiDepth);
  }

  return NS_SUCCESS;
}

nsResult nsDdsFileFormat::ReadImageHeader(nsStreamReader& inout_stream, nsImageHeader& ref_header, nsStringView sFileExtension) const
{
  NS_PROFILE_SCOPE("nsDdsFileFormat::ReadImageHeader");

  nsDdsHeader ddsHeader;
  return ReadImageData(inout_stream, ref_header, ddsHeader);
}

nsResult nsDdsFileFormat::ReadImage(nsStreamReader& inout_stream, nsImage& ref_image, nsStringView sFileExtension) const
{
  NS_PROFILE_SCOPE("nsDdsFileFormat::ReadImage");

  nsImageHeader imageHeader;
  nsDdsHeader ddsHeader;
  NS_SUCCEED_OR_RETURN(ReadImageData(inout_stream, imageHeader, ddsHeader));

  ref_image.ResetAndAlloc(imageHeader);

  const bool bPitch = (ddsHeader.m_uiFlags & nsDdsdFlags::PITCH) != 0;

  // If pitch is specified, it must match the computed value
  if (bPitch && ref_image.GetRowPitch(0) != ddsHeader.m_uiPitchOrLinearSize)
  {
    nsLog::Error("The row pitch specified in the header doesn't match the expected pitch.");
    return NS_FAILURE;
  }

  nsUInt64 uiDataSize = ref_image.GetByteBlobPtr().GetCount();

  if (inout_stream.ReadBytes(ref_image.GetByteBlobPtr().GetPtr(), uiDataSize) != uiDataSize)
  {
    nsLog::Error("Failed to read image data.");
    return NS_FAILURE;
  }

  return NS_SUCCESS;
}

nsResult nsDdsFileFormat::WriteImage(nsStreamWriter& inout_stream, const nsImageView& image, nsStringView sFileExtension) const
{
  const nsImageFormat::Enum format = image.GetImageFormat();
  const nsUInt32 uiBpp = nsImageFormat::GetBitsPerPixel(format);

  const nsUInt32 uiNumFaces = image.GetNumFaces();
  const nsUInt32 uiNumMipLevels = image.GetNumMipLevels();
  const nsUInt32 uiNumArrayIndices = image.GetNumArrayIndices();

  const nsUInt32 uiWidth = image.GetWidth(0);
  const nsUInt32 uiHeight = image.GetHeight(0);
  const nsUInt32 uiDepth = image.GetDepth(0);

  bool bHasMipMaps = uiNumMipLevels > 1;
  bool bVolume = uiDepth > 1;
  bool bCubeMap = uiNumFaces > 1;
  bool bArray = uiNumArrayIndices > 1;

  bool bDxt10 = false;

  nsDdsHeader fileHeader;
  nsDdsHeaderDxt10 headerDxt10;

  nsMemoryUtils::ZeroFill(&fileHeader, 1);
  nsMemoryUtils::ZeroFill(&headerDxt10, 1);

  fileHeader.m_uiMagic = nsDdsMagic;
  fileHeader.m_uiSize = 124;
  fileHeader.m_uiWidth = uiWidth;
  fileHeader.m_uiHeight = uiHeight;

  // Required in every .dds file.
  fileHeader.m_uiFlags = nsDdsdFlags::WIDTH | nsDdsdFlags::HEIGHT | nsDdsdFlags::CAPS | nsDdsdFlags::PIXELFORMAT;

  if (bHasMipMaps)
  {
    fileHeader.m_uiFlags |= nsDdsdFlags::MIPMAPCOUNT;
    fileHeader.m_uiMipMapCount = uiNumMipLevels;
  }

  if (bVolume)
  {
    // Volume and array are incompatible
    if (bArray)
    {
      nsLog::Error("The image is both an array and volume texture. This is not supported.");
      return NS_FAILURE;
    }

    fileHeader.m_uiFlags |= nsDdsdFlags::DEPTH;
    fileHeader.m_uiDepth = uiDepth;
  }

  switch (nsImageFormat::GetType(image.GetImageFormat()))
  {
    case nsImageFormatType::LINEAR:
      [[fallthrough]];
    case nsImageFormatType::PLANAR:
      fileHeader.m_uiFlags |= nsDdsdFlags::PITCH;
      fileHeader.m_uiPitchOrLinearSize = static_cast<nsUInt32>(image.GetRowPitch(0));
      break;

    case nsImageFormatType::BLOCK_COMPRESSED:
      fileHeader.m_uiFlags |= nsDdsdFlags::LINEARSIZE;
      fileHeader.m_uiPitchOrLinearSize = 0; /// \todo sub-image size
      break;

    default:
      nsLog::Error("Unknown image format type.");
      return NS_FAILURE;
  }

  fileHeader.m_uiCaps = nsDdsCaps::TEXTURE;

  if (bCubeMap)
  {
    if (uiNumFaces != 6)
    {
      nsLog::Error("The image is a cubemap, but has {0} faces instead of the expected 6.", uiNumFaces);
      return NS_FAILURE;
    }

    if (bVolume)
    {
      nsLog::Error("The image is both a cubemap and volume texture. This is not supported.");
      return NS_FAILURE;
    }

    fileHeader.m_uiCaps |= nsDdsCaps::COMPLEX;
    fileHeader.m_uiCaps2 |= nsDdsCaps2::CUBEMAP | nsDdsCaps2::CUBEMAP_POSITIVEX | nsDdsCaps2::CUBEMAP_NEGATIVEX | nsDdsCaps2::CUBEMAP_POSITIVEY |
                            nsDdsCaps2::CUBEMAP_NEGATIVEY | nsDdsCaps2::CUBEMAP_POSITIVNS | nsDdsCaps2::CUBEMAP_NEGATIVNS;
  }

  if (bArray)
  {
    fileHeader.m_uiCaps |= nsDdsCaps::COMPLEX;

    // Must be written as DXT10
    bDxt10 = true;
  }

  if (bVolume)
  {
    fileHeader.m_uiCaps |= nsDdsCaps::COMPLEX;
    fileHeader.m_uiCaps2 |= nsDdsCaps2::VOLUME;
  }

  if (bHasMipMaps)
  {
    fileHeader.m_uiCaps |= nsDdsCaps::MIPMAP | nsDdsCaps::COMPLEX;
  }

  fileHeader.m_ddspf.m_uiSize = 32;

  nsUInt32 uiRedMask = nsImageFormat::GetRedMask(format);
  nsUInt32 uiGreenMask = nsImageFormat::GetGreenMask(format);
  nsUInt32 uiBlueMask = nsImageFormat::GetBlueMask(format);
  nsUInt32 uiAlphaMask = nsImageFormat::GetAlphaMask(format);

  nsUInt32 uiFourCc = nsImageFormatMappings::ToFourCc(format);
  nsUInt32 uiDxgiFormat = nsImageFormatMappings::ToDxgiFormat(format);

  // When not required to use a DXT10 texture, try to write a legacy DDS by specifying FourCC or pixel masks
  if (!bDxt10)
  {
    // The format has a known mask and we would also recognize it as the same when reading back in, since multiple formats may have the same pixel
    // masks
    if ((uiRedMask | uiGreenMask | uiBlueMask | uiAlphaMask) &&
        format == nsImageFormat::FromPixelMask(uiRedMask, uiGreenMask, uiBlueMask, uiAlphaMask, uiBpp))
    {
      fileHeader.m_ddspf.m_uiFlags = nsDdpfFlags::ALPHAPIXELS | nsDdpfFlags::RGB;
      fileHeader.m_ddspf.m_uiRBitMask = uiRedMask;
      fileHeader.m_ddspf.m_uiGBitMask = uiGreenMask;
      fileHeader.m_ddspf.m_uiBBitMask = uiBlueMask;
      fileHeader.m_ddspf.m_uiABitMask = uiAlphaMask;
      fileHeader.m_ddspf.m_uiRGBBitCount = nsImageFormat::GetBitsPerPixel(format);
    }
    // The format has a known FourCC
    else if (uiFourCc != 0)
    {
      fileHeader.m_ddspf.m_uiFlags = nsDdpfFlags::FOURCC;
      fileHeader.m_ddspf.m_uiFourCC = uiFourCc;
    }
    else
    {
      // Fallback to DXT10 path
      bDxt10 = true;
    }
  }

  if (bDxt10)
  {
    // We must write a DXT10 file, but there is no matching DXGI_FORMAT - we could also try converting, but that is rarely intended when writing .dds
    if (uiDxgiFormat == 0)
    {
      nsLog::Error("The image needs to be written as a DXT10 file, but no matching DXGI format was found for '{0}'.", nsImageFormat::GetName(format));
      return NS_FAILURE;
    }

    fileHeader.m_ddspf.m_uiFlags = nsDdpfFlags::FOURCC;
    fileHeader.m_ddspf.m_uiFourCC = nsDdsDxt10FourCc;

    headerDxt10.m_uiDxgiFormat = uiDxgiFormat;

    if (bVolume)
    {
      headerDxt10.m_uiResourceDimension = nsDdsResourceDimension::TEXTURE3D;
    }
    else if (uiHeight > 1)
    {
      headerDxt10.m_uiResourceDimension = nsDdsResourceDimension::TEXTURE2D;
    }
    else
    {
      headerDxt10.m_uiResourceDimension = nsDdsResourceDimension::TEXTURE1D;
    }

    if (bCubeMap)
    {
      headerDxt10.m_uiMiscFlag = nsDdsResourceMiscFlags::TEXTURECUBE;
    }

    // NOT multiplied by number of cubemap faces
    headerDxt10.m_uiArraySize = uiNumArrayIndices;

    // Can be used to describe the alpha channel usage, but automatically makes it incompatible with the D3DX libraries if not 0.
    headerDxt10.m_uiMiscFlags2 = 0;
  }

  if (inout_stream.WriteBytes(&fileHeader, sizeof(fileHeader)) != NS_SUCCESS)
  {
    nsLog::Error("Failed to write image header.");
    return NS_FAILURE;
  }

  if (bDxt10)
  {
    if (inout_stream.WriteBytes(&headerDxt10, sizeof(headerDxt10)) != NS_SUCCESS)
    {
      nsLog::Error("Failed to write image DX10 header.");
      return NS_FAILURE;
    }
  }

  if (inout_stream.WriteBytes(image.GetByteBlobPtr().GetPtr(), image.GetByteBlobPtr().GetCount()) != NS_SUCCESS)
  {
    nsLog::Error("Failed to write image data.");
    return NS_FAILURE;
  }

  return NS_SUCCESS;
}

bool nsDdsFileFormat::CanReadFileType(nsStringView sExtension) const
{
  return sExtension.IsEqual_NoCase("dds");
}

bool nsDdsFileFormat::CanWriteFileType(nsStringView sExtension) const
{
  return CanReadFileType(sExtension);
}



NS_STATICLINK_FILE(Texture, Texture_Image_Formats_DdsFileFormat);
