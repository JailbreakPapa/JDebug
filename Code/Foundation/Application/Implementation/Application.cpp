#include <Foundation/FoundationPCH.h>

#include <Foundation/Application/Application.h>
#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/System/SystemInformation.h>
#include <Foundation/Threading/ThreadUtils.h>
#include <Foundation/Utilities/CommandLineOptions.h>

wdApplication::wdApplication(const char* szAppName)
  : m_iReturnCode(0)
  , m_uiArgumentCount(0)
  , m_pArguments(nullptr)
  , m_bReportMemoryLeaks(true)
  , m_sAppName(szAppName)
{
}

wdApplication::~wdApplication() = default;

void wdApplication::SetApplicationName(const char* szAppName)
{
  m_sAppName = szAppName;
}

wdCommandLineOptionBool opt_WaitForDebugger("app", "-WaitForDebugger", "If specified, the application will wait at startup until a debugger is attached.", false);

wdResult wdApplication::BeforeCoreSystemsStartup()
{
  if (wdFileSystem::DetectSdkRootDirectory().Failed())
  {
    wdLog::Error("Unable to find the SDK root directory. Mounting data directories may fail.");
  }

#if WD_ENABLED(WD_COMPILE_FOR_DEBUG)
  wdRTTI::VerifyCorrectnessForAllTypes();
#endif

  if (opt_WaitForDebugger.GetOptionValue(wdCommandLineOption::LogMode::AlwaysIfSpecified))
  {
    while (!wdSystemInformation::IsDebuggerAttached())
    {
      wdThreadUtils::Sleep(wdTime::Milliseconds(1));
    }

    WD_DEBUG_BREAK;
  }

  return WD_SUCCESS;
}


void wdApplication::SetCommandLineArguments(wdUInt32 uiArgumentCount, const char** pArguments)
{
  m_uiArgumentCount = uiArgumentCount;
  m_pArguments = pArguments;

  wdCommandLineUtils::GetGlobalInstance()->SetCommandLine(uiArgumentCount, pArguments, wdCommandLineUtils::PreferOsArgs);
}


const char* wdApplication::GetArgument(wdUInt32 uiArgument) const
{
  WD_ASSERT_DEV(uiArgument < m_uiArgumentCount, "There are only {0} arguments, cannot access argument {1}.", m_uiArgumentCount, uiArgument);

  return m_pArguments[uiArgument];
}


void wdApplication::RequestQuit()
{
  m_bWasQuitRequested = true;
}


wdApplication* wdApplication::s_pApplicationInstance = nullptr;



WD_STATICLINK_FILE(Foundation, Foundation_Application_Implementation_Application);
