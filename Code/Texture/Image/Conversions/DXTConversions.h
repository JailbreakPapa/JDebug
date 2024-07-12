#pragma once

#include <Texture/Image/Image.h>

class nsColorLinear16f;

NS_TEXTURE_DLL void nsDecompressBlockBC1(const nsUInt8* pSource, nsColorBaseUB* pTarget, bool bForceFourColorMode);
NS_TEXTURE_DLL void nsDecompressBlockBC4(const nsUInt8* pSource, nsUInt8* pTarget, nsUInt32 uiStride, nsUInt8 uiBias);
NS_TEXTURE_DLL void nsDecompressBlockBC6(const nsUInt8* pSource, nsColorLinear16f* pTarget, bool bIsSigned);
NS_TEXTURE_DLL void nsDecompressBlockBC7(const nsUInt8* pSource, nsColorBaseUB* pTarget);

NS_TEXTURE_DLL void nsUnpackPaletteBC4(nsUInt32 ui0, nsUInt32 ui1, nsUInt32* pAlphas);
