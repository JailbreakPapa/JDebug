#include <Foundation/FoundationPCH.h>

#include <Foundation/Application/Application.h>
#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/System/SystemInformation.h>
#include <Foundation/Threading/ThreadUtils.h>
#include <Foundation/Utilities/CommandLineOptions.h>

nsApplication::nsApplication(nsStringView sAppName)
  : m_sAppName(sAppName)
{
}

nsApplication::~nsApplication() = default;

void nsApplication::SetApplicationName(nsStringView sAppName)
{
  m_sAppName = sAppName;
}

nsCommandLineOptionBool opt_WaitForDebugger("app", "-WaitForDebugger", "If specified, the application will wait at startup until a debugger is attached.", false);

nsResult nsApplication::BeforeCoreSystemsStartup()
{
  if (nsFileSystem::DetectSdkRootDirectory().Failed())
  {
    nsLog::Error("Unable to find the SDK root directory. Mounting data directories may fail.");
  }

#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
  nsRTTI::VerifyCorrectnessForAllTypes();
#endif

  if (opt_WaitForDebugger.GetOptionValue(nsCommandLineOption::LogMode::AlwaysIfSpecified))
  {
    while (!nsSystemInformation::IsDebuggerAttached())
    {
      nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(1));
    }

    NS_DEBUG_BREAK;
  }

  return NS_SUCCESS;
}


void nsApplication::SetCommandLineArguments(nsUInt32 uiArgumentCount, const char** pArguments)
{
  m_uiArgumentCount = uiArgumentCount;
  m_pArguments = pArguments;

  nsCommandLineUtils::GetGlobalInstance()->SetCommandLine(uiArgumentCount, pArguments, nsCommandLineUtils::PreferOsArgs);
}


const char* nsApplication::GetArgument(nsUInt32 uiArgument) const
{
  NS_ASSERT_DEV(uiArgument < m_uiArgumentCount, "There are only {0} arguments, cannot access argument {1}.", m_uiArgumentCount, uiArgument);

  return m_pArguments[uiArgument];
}


void nsApplication::RequestQuit()
{
  m_bWasQuitRequested = true;
}


nsApplication* nsApplication::s_pApplicationInstance = nullptr;
