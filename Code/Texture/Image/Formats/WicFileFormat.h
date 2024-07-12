#pragma once

#include <Texture/Image/Formats/ImageFileFormat.h>

#if NS_ENABLED(NS_PLATFORM_WINDOWS_DESKTOP)

/// \brief File format implementation for loading TIFF files using WIC
class NS_TEXTURE_DLL nsWicFileFormat : public nsImageFileFormat
{
public:
  nsWicFileFormat();
  virtual ~nsWicFileFormat();

  virtual nsResult ReadImageHeader(nsStreamReader& inout_stream, nsImageHeader& ref_header, nsStringView sFileExtension) const override;
  virtual nsResult ReadImage(nsStreamReader& inout_stream, nsImage& ref_image, nsStringView sFileExtension) const override;
  virtual nsResult WriteImage(nsStreamWriter& inout_stream, const nsImageView& image, nsStringView sFileExtension) const override;

  virtual bool CanReadFileType(nsStringView sExtension) const override;
  virtual bool CanWriteFileType(nsStringView sExtension) const override;

private:
  mutable bool m_bTryCoInit = true; // Helper for keeping track of whether we have tried to init COM exactly once
  mutable bool m_bCoUninitOnShutdown =
    false;                          // Helper for keeping track of whether we have to uninitialize COM (because we were the first to initialize it)

  nsResult ReadFileData(nsStreamReader& stream, nsDynamicArray<nsUInt8>& storage) const;
};

#endif
