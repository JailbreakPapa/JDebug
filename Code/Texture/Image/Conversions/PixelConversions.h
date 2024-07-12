#pragma once

#include <Texture/Image/Image.h>

NS_TEXTURE_DLL nsColorBaseUB nsDecompressA4B4G4R4(nsUInt16 uiColor);
NS_TEXTURE_DLL nsColorBaseUB nsDecompressB4G4R4A4(nsUInt16 uiColor);
NS_TEXTURE_DLL nsColorBaseUB nsDecompressB5G6R5(nsUInt16 uiColor);
NS_TEXTURE_DLL nsColorBaseUB nsDecompressB5G5R5X1(nsUInt16 uiColor);
NS_TEXTURE_DLL nsColorBaseUB nsDecompressB5G5R5A1(nsUInt16 uiColor);
NS_TEXTURE_DLL nsColorBaseUB nsDecompressX1B5G5R5(nsUInt16 uiColor);
NS_TEXTURE_DLL nsColorBaseUB nsDecompressA1B5G5R5(nsUInt16 uiColor);
NS_TEXTURE_DLL nsUInt16 nsCompressA4B4G4R4(nsColorBaseUB color);
NS_TEXTURE_DLL nsUInt16 nsCompressB4G4R4A4(nsColorBaseUB color);
NS_TEXTURE_DLL nsUInt16 nsCompressB5G6R5(nsColorBaseUB color);
NS_TEXTURE_DLL nsUInt16 nsCompressB5G5R5X1(nsColorBaseUB color);
NS_TEXTURE_DLL nsUInt16 nsCompressB5G5R5A1(nsColorBaseUB color);
NS_TEXTURE_DLL nsUInt16 nsCompressX1B5G5R5(nsColorBaseUB color);
NS_TEXTURE_DLL nsUInt16 nsCompressA1B5G5R5(nsColorBaseUB color);
