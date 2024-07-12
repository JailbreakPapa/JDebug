#include <TestFramework/TestFrameworkPCH.h>

#include <TestFramework/Utilities/TestSetup.h>

#include <TestFramework/Utilities/ConsoleOutput.h>
#include <TestFramework/Utilities/HTMLOutput.h>

#include <Foundation/System/CrashHandler.h>
#include <Foundation/System/SystemInformation.h>

#ifdef NS_USE_QT
#  include <TestFramework/Framework/Qt/qtTestFramework.h>
#  include <TestFramework/Framework/Qt/qtTestGUI.h>
#elif NS_ENABLED(NS_PLATFORM_WINDOWS_UWP)
#  include <TestFramework/Framework/Uwp/uwpTestFramework.h>
#endif

#if NS_ENABLED(NS_PLATFORM_WINDOWS)
#  include <conio.h>
#endif

int nsTestSetup::s_iArgc = 0;
const char** nsTestSetup::s_pArgv = nullptr;

nsTestFramework* nsTestSetup::InitTestFramework(const char* szTestName, const char* szNiceTestName, int iArgc, const char** pArgv)
{
  s_iArgc = iArgc;
  s_pArgv = pArgv;

#if NS_ENABLED(NS_PLATFORM_WINDOWS_UWP)
  if (FAILED(RoInitialize(RO_INIT_MULTITHREADED)))
  {
    std::cout << "Failed to init WinRT." << std::endl;
  }
#endif

  // without a proper file system the current working directory is pretty much useless
  std::string sTestFolder = std::string(nsOSFile::GetUserDataFolder());
  if (*sTestFolder.rbegin() != '/')
    sTestFolder.append("/");
  sTestFolder.append("nsEngine Tests/");
  sTestFolder.append(szTestName);

  std::string sTestDataSubFolder = "Data/UnitTests/";
  sTestDataSubFolder.append(szTestName);

#ifdef NS_USE_QT
  nsTestFramework* pTestFramework = new nsQtTestFramework(szNiceTestName, sTestFolder.c_str(), sTestDataSubFolder.c_str(), iArgc, pArgv);
#elif NS_ENABLED(NS_PLATFORM_WINDOWS_UWP)
  // Command line args in UWP are handled differently and can't be retrieved from the main function.
  nsTestFramework* pTestFramework = new nsUwpTestFramework(szNiceTestName, sTestFolder.c_str(), sTestDataSubFolder.c_str(), 0, nullptr);
#else
  nsTestFramework* pTestFramework = new nsTestFramework(szNiceTestName, sTestFolder.c_str(), sTestDataSubFolder.c_str(), iArgc, pArgv);
#endif

  // Register some output handlers to forward all the messages to the console and to an HTML file
  pTestFramework->RegisterOutputHandler(OutputToConsole);
  pTestFramework->RegisterOutputHandler(nsOutputToHTML::OutputToHTML);

  nsCrashHandler_WriteMiniDump::g_Instance.SetDumpFilePath(pTestFramework->GetAbsOutputPath(), szTestName);
  nsCrashHandler::SetCrashHandler(&nsCrashHandler_WriteMiniDump::g_Instance);

  return pTestFramework;
}

nsTestAppRun nsTestSetup::RunTests()
{
  nsTestFramework* pTestFramework = nsTestFramework::GetInstance();

  // Todo: Incorporate all the below in a virtual call of testFramework?
#ifdef NS_USE_QT
  TestSettings settings = pTestFramework->GetSettings();
  if (settings.m_bNoGUI)
  {
    return pTestFramework->RunTestExecutionLoop();
  }

  // Setup Qt Application

  int argc = s_iArgc;
  char** argv = const_cast<char**>(s_pArgv);

  if (qApp != nullptr)
  {
    bool ok = false;
    int iCount = qApp->property("Shared").toInt(&ok);
    NS_ASSERT_DEV(ok, "Existing QApplication was not constructed by NS!");
    qApp->setProperty("Shared", QVariant::fromValue(iCount + 1));
  }
  else
  {
    new QApplication(argc, argv);
    qApp->setProperty("Shared", QVariant::fromValue((int)1));
    qApp->setApplicationName(pTestFramework->GetTestName());
    nsQtTestGUI::SetDarkTheme();
  }

  // Create main window
  {
    nsQtTestGUI mainWindow(*static_cast<nsQtTestFramework*>(pTestFramework));
    mainWindow.show();

    qApp->exec();
  }
  {
    const int iCount = qApp->property("Shared").toInt();
    if (iCount == 1)
    {
      delete qApp;
    }
    else
    {
      qApp->setProperty("Shared", QVariant::fromValue(iCount - 1));
    }
  }

  return nsTestAppRun::Quit;
#elif NS_ENABLED(NS_PLATFORM_WINDOWS_UWP)
  static_cast<nsUwpTestFramework*>(pTestFramework)->Run();
  return nsTestAppRun::Quit;
#else
  // Run all the tests with the given order
  return pTestFramework->RunTestExecutionLoop();
#endif
}

void nsTestSetup::DeInitTestFramework(bool bSilent /*= false*/)
{
  nsTestFramework* pTestFramework = nsTestFramework::GetInstance();

  nsStartup::ShutdownCoreSystems();

  // In the UWP case we never initialized this thread for ns, so we can't do log output now.
#if NS_DISABLED(NS_PLATFORM_WINDOWS_UWP)
  if (!bSilent)
  {
    nsGlobalLog::AddLogWriter(nsLogWriter::Console::LogMessageHandler);
  }
#endif

  TestSettings settings = pTestFramework->GetSettings();
  if (settings.m_bKeepConsoleOpen && !bSilent)
  {
    if (nsSystemInformation::IsDebuggerAttached())
    {
      std::cout << "Press the any key to continue...\n";
      fflush(stdin);
      [[maybe_unused]] int c = getchar();
    }
  }

  // This is needed as at least windows can't be bothered to write anything
  // to the output streams at all if it's not enough or the app is too fast.
  fflush(stdout);
  fflush(stderr);
  delete pTestFramework;
}

nsInt32 nsTestSetup::GetFailedTestCount()
{
  return nsTestFramework::GetInstance()->GetTestsFailedCount();
}
