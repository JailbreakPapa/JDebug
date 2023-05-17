#include <Foundation/FoundationPCH.h>

#include <Foundation/Basics/Platform/Win/IncludeWindows.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Strings/StringBuilder.h>
#include <Foundation/Strings/StringUtils.h>
#include <Foundation/System/EnvironmentVariableUtils.h>
#include <Foundation/System/SystemInformation.h>
#include <Foundation/Utilities/ConversionUtils.h>

#include <cstdio>
#include <cstdlib>
#include <ctime>

#if WD_ENABLED(WD_PLATFORM_WINDOWS) && WD_ENABLED(WD_COMPILE_FOR_DEVELOPMENT)
#  include <crtdbg.h>
#endif

#if WD_ENABLED(WD_COMPILER_MSVC)
void MSVC_OutOfLine_DebugBreak(...)
{
  __debugbreak();
}
#endif

bool wdDefaultAssertHandler(const char* szSourceFile, wdUInt32 uiLine, const char* szFunction, const char* szExpression, const char* szAssertMsg)
{
  char szTemp[1024 * 4] = "";
  wdStringUtils::snprintf(szTemp, WD_ARRAY_SIZE(szTemp),
    "\n\n *** Assertion ***\n\n    Expression: \"%s\"\n    Function: \"%s\"\n    File: \"%s\"\n    Line: %u\n    Message: \"%s\"\n\n", szExpression,
    szFunction, szSourceFile, uiLine, szAssertMsg);
  szTemp[1024 * 4 - 1] = '\0';

  wdLog::Print(szTemp);

  if (wdSystemInformation::IsDebuggerAttached())
    return true;

  // If no debugger is attached we append the assert to a common file so that postmortem debugging is easier
  if (FILE* assertLogFP = fopen("wdDefaultAssertHandlerOutput.txt", "a"))
  {
    time_t timeUTC = time(&timeUTC);
    tm* ptm = gmtime(&timeUTC);

    char szTimeStr[256] = {0};
    sprintf(szTimeStr, "UTC: %s", asctime(ptm));
    fputs(szTimeStr, assertLogFP);

    fputs(szTemp, assertLogFP);

    fclose(assertLogFP);
  }

  // if the environment variable "WD_SILENT_ASSERTS" is set to a value like "1", "on", "true", "enable" or "yes"
  // the assert handler will never show a GUI that may block the application from continuing to run
  // this should be set on machines that run tests which should never get stuck but rather crash asap
  bool bSilentAsserts = false;

  if (wdEnvironmentVariableUtils::IsVariableSet("WD_SILENT_ASSERTS"))
  {
    bSilentAsserts = wdEnvironmentVariableUtils::GetValueInt("WD_SILENT_ASSERTS", bSilentAsserts ? 1 : 0) != 0;
  }

  if (bSilentAsserts)
    return true;

#if WD_ENABLED(WD_PLATFORM_WINDOWS)

    // make sure the cursor is definitely shown, since the user must be able to click buttons
#  if WD_ENABLED(WD_PLATFORM_WINDOWS_UWP)
    // Todo: Use modern Windows API to show cursor in current window.
    // http://stackoverflow.com/questions/37956628/change-mouse-pointer-in-uwp-app
#  else
  wdInt32 iHideCursor = 1;
  while (ShowCursor(true) < 0)
    ++iHideCursor;
#  endif

#  if WD_ENABLED(WD_COMPILE_FOR_DEBUG)

  wdInt32 iRes = _CrtDbgReport(_CRT_ASSERT, szSourceFile, uiLine, nullptr, "'%s'\nFunction: %s\nMessage: %s", szExpression, szFunction, szAssertMsg);

  // currently we will ALWAYS trigger the breakpoint / crash (except for when the user presses 'ignore')
  if (iRes == 0)
  {
    // when the user ignores the assert, restore the cursor show/hide state to the previous count
#    if WD_ENABLED(WD_PLATFORM_WINDOWS_UWP)
    // Todo: Use modern Windows API to restore cursor.
#    else
    for (wdInt32 i = 0; i < iHideCursor; ++i)
      ShowCursor(false);
#    endif

    return false;
  }

#  else


#    if WD_ENABLED(WD_PLATFORM_WINDOWS_DESKTOP)
  MessageBoxA(nullptr, szTemp, "Assertion", MB_ICONERROR);
#    endif

#  endif

#endif

  // always do a debug-break
  // in release-builds this will just crash the app
  return true;
}

static wdAssertHandler g_AssertHandler = &wdDefaultAssertHandler;

wdAssertHandler wdGetAssertHandler()
{
  return g_AssertHandler;
}

void wdSetAssertHandler(wdAssertHandler handler)
{
  g_AssertHandler = handler;
}

bool wdFailedCheck(const char* szSourceFile, wdUInt32 uiLine, const char* szFunction, const char* szExpression, const char* szMsg)
{
  // always do a debug-break if no assert handler is installed
  if (g_AssertHandler == nullptr)
    return true;

  return (*g_AssertHandler)(szSourceFile, uiLine, szFunction, szExpression, szMsg);
}

bool wdFailedCheck(const char* szSourceFile, wdUInt32 uiLine, const char* szFunction, const char* szExpression, const class wdFormatString& msg)
{
  wdStringBuilder tmp;
  return wdFailedCheck(szSourceFile, uiLine, szFunction, szExpression, msg.GetText(tmp));
}


WD_STATICLINK_FILE(Foundation, Foundation_Basics_Assert);
