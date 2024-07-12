#include <Foundation/Platform/PlatformDesc.h>

nsPlatformDesc g_PlatformDescUWP("UWP");

#if NS_ENABLED(NS_PLATFORM_WINDOWS_UWP)

const nsPlatformDesc* nsPlatformDesc::s_pThisPlatform = &g_PlatformDescUWP;

#endif
