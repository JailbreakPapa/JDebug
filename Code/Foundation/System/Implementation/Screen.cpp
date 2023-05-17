#include <Foundation/FoundationPCH.h>

#include <Foundation/System/PlatformFeatures.h>
#include <Foundation/System/Screen.h>

#if WD_ENABLED(WD_SUPPORTS_GLFW)
#  include <Foundation/System/Implementation/glfw/Screen_glfw.inl>
#elif WD_ENABLED(WD_PLATFORM_WINDOWS_DESKTOP)
#  include <Foundation/System/Implementation/Win/Screen_win32.inl>
#elif WD_ENABLED(WD_PLATFORM_WINDOWS_UWP)
#  include <Foundation/System/Implementation/uwp/Screen_uwp.inl>
#else

wdResult wdScreen::EnumerateScreens(wdHybridArray<wdScreenInfo, 2>& out_Screens)
{
  WD_ASSERT_NOT_IMPLEMENTED;
  return WD_FAILURE;
}

#endif

void wdScreen::PrintScreenInfo(const wdHybridArray<wdScreenInfo, 2>& screens, wdLogInterface* pLog /*= wdLog::GetThreadLocalLogSystem()*/)
{
  WD_LOG_BLOCK(pLog, "Screens");

  wdLog::Info(pLog, "Found {0} screens", screens.GetCount());

  for (const auto& screen : screens)
  {
    wdLog::Dev(pLog, "'{0}': Offset = ({1}, {2}), Resolution = ({3}, {4}){5}", screen.m_sDisplayName, screen.m_iOffsetX, screen.m_iOffsetY, screen.m_iResolutionX, screen.m_iResolutionY, screen.m_bIsPrimary ? " (primary)" : "");
  }
}


WD_STATICLINK_FILE(Foundation, Foundation_System_Implementation_Screen);
