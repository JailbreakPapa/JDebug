#pragma once

#include <Texture/Image/Formats/ImageFileFormat.h>

#if NS_DISABLED(NS_PLATFORM_WINDOWS_UWP)

/// EXR file format support using TinyEXR.
class NS_TEXTURE_DLL nsExrFileFormat : public nsImageFileFormat
{
public:
  nsResult ReadImageHeader(nsStreamReader& ref_stream, nsImageHeader& ref_header, nsStringView sFileExtension) const override;
  nsResult ReadImage(nsStreamReader& ref_stream, nsImage& ref_image, nsStringView sFileExtension) const override;
  nsResult WriteImage(nsStreamWriter& ref_stream, const nsImageView& image, nsStringView sFileExtension) const override;

  bool CanReadFileType(nsStringView sExtension) const override;
  bool CanWriteFileType(nsStringView sExtension) const override;
};

#endif
