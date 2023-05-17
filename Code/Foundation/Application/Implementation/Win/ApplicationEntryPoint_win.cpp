#include <Foundation/FoundationPCH.h>

#if WD_ENABLED(WD_PLATFORM_WINDOWS_DESKTOP)
#  include <Foundation/Application/Implementation/Win/ApplicationEntryPoint_win.h>
#  include <Foundation/Basics/Platform/Win/IncludeWindows.h>

namespace wdApplicationDetails
{
  void SetConsoleCtrlHandler(wdMinWindows::BOOL(WD_WINDOWS_WINAPI* consoleHandler)(wdMinWindows::DWORD dwCtrlType))
  {
    ::SetConsoleCtrlHandler(consoleHandler, TRUE);
  }

  static wdMutex s_shutdownMutex;

  wdMutex& GetShutdownMutex()
  {
    return s_shutdownMutex;
  }

} // namespace wdApplicationDetails
#endif


WD_STATICLINK_FILE(Foundation, Foundation_Application_Implementation_Win_ApplicationEntryPoint_win);
