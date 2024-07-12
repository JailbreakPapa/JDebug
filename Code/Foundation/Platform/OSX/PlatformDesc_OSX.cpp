#include <Foundation/Platform/PlatformDesc.h>

nsPlatformDesc g_PlatformDescOSX("OSX");

#if NS_ENABLED(NS_PLATFORM_OSX)

const nsPlatformDesc* nsPlatformDesc::s_pThisPlatform = &g_PlatformDescOSX;

#endif
