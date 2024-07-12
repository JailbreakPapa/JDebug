#include <Foundation/FoundationPCH.h>

#include <Foundation/Basics/Platform/PlatformFeatures.h>
#include <Foundation/System/Screen.h>

#if NS_ENABLED(NS_PLATFORM_ANDROID)

nsResult nsScreen::EnumerateScreens(nsDynamicArray<nsScreenInfo>& out_Screens)
{
  if (ANativeWindow* pWindow = nsAndroidUtils::GetAndroidApp()->window)
  {
    nsScreenInfo& currentScreen = out_Screens.ExpandAndGetRef();
    currentScreen.m_sDisplayName = "Current Display";
    currentScreen.m_iOffsetX = 0;
    currentScreen.m_iOffsetY = 0;
    currentScreen.m_iResolutionX = ANativeWindow_getWidth(pWindow);
    currentScreen.m_iResolutionY = ANativeWindow_getHeight(pWindow);
    currentScreen.m_bIsPrimary = true;
    return NS_SUCCESS;
  }
  return NS_FAILURE;
}
#endif
