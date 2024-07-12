#include <Texture/TexturePCH.h>

NS_STATICLINK_LIBRARY(Texture)
{
  if (bReturn)
    return;

  NS_STATICLINK_REFERENCE(Texture_Image_Conversions_DXTConversions);
  NS_STATICLINK_REFERENCE(Texture_Image_Conversions_DXTexConversions);
  NS_STATICLINK_REFERENCE(Texture_Image_Conversions_DXTexCpuConversions);
  NS_STATICLINK_REFERENCE(Texture_Image_Conversions_PixelConversions);
  NS_STATICLINK_REFERENCE(Texture_Image_Conversions_PlanarConversions);
  NS_STATICLINK_REFERENCE(Texture_Image_Formats_BmpFileFormat);
  NS_STATICLINK_REFERENCE(Texture_Image_Formats_DdsFileFormat);
  NS_STATICLINK_REFERENCE(Texture_Image_Formats_ExrFileFormat);
  NS_STATICLINK_REFERENCE(Texture_Image_Formats_StbImageFileFormats);
  NS_STATICLINK_REFERENCE(Texture_Image_Formats_TgaFileFormat);
  NS_STATICLINK_REFERENCE(Texture_Image_Formats_WicFileFormat);
  NS_STATICLINK_REFERENCE(Texture_Image_Implementation_ImageEnums);
  NS_STATICLINK_REFERENCE(Texture_Image_Implementation_ImageFormat);
  NS_STATICLINK_REFERENCE(Texture_TexConv_Implementation_Processor);
}
