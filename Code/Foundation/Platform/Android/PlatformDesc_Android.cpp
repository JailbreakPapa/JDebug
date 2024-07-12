#include <Foundation/Platform/PlatformDesc.h>

nsPlatformDesc g_PlatformDescAndroid("Android");

#if NS_ENABLED(NS_PLATFORM_ANDROID)

const nsPlatformDesc* nsPlatformDesc::s_pThisPlatform = &g_PlatformDescAndroid;

#endif
