#pragma once

#include <Texture/Image/Formats/ImageFileFormat.h>

class NS_TEXTURE_DLL nsBmpFileFormat : public nsImageFileFormat
{
public:
  virtual nsResult ReadImageHeader(nsStreamReader& inout_stream, nsImageHeader& ref_header, nsStringView sFileExtension) const override;

  virtual nsResult ReadImage(nsStreamReader& inout_stream, nsImage& ref_image, nsStringView sFileExtension) const override;
  virtual nsResult WriteImage(nsStreamWriter& inout_stream, const nsImageView& image, nsStringView sFileExtension) const override;

  virtual bool CanReadFileType(nsStringView sExtension) const override;
  virtual bool CanWriteFileType(nsStringView sExtension) const override;
};
