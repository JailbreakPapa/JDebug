#include <TestFramework/TestFrameworkPCH.h>

#include <Texture/Image/Formats/ImageFileFormat.h>

#include <Foundation/Basics/Platform/Win/IncludeWindows.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/MemoryStream.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Logging/VisualStudioWriter.h>
#include <Foundation/System/CrashHandler.h>
#include <Foundation/System/EnvironmentVariableUtils.h>
#include <Foundation/System/Process.h>
#include <Foundation/System/StackTracer.h>
#include <Foundation/Threading/ThreadUtils.h>
#include <Foundation/Types/ScopeExit.h>
#include <Foundation/Utilities/CommandLineOptions.h>
#include <TestFramework/Utilities/TestOrder.h>

#include <cstdlib>
#include <stdexcept>
#include <stdlib.h>

#ifdef NS_TESTFRAMEWORK_USE_FILESERVE
#  include <FileservePlugin/Client/FileserveClient.h>
#  include <FileservePlugin/Client/FileserveDataDir.h>
#  include <FileservePlugin/FileservePluginDLL.h>
#endif

nsTestFramework* nsTestFramework::s_pInstance = nullptr;

const char* nsTestFramework::s_szTestBlockName = "";
int nsTestFramework::s_iAssertCounter = 0;
bool nsTestFramework::s_bCallstackOnAssert = false;
nsLog::TimestampMode nsTestFramework::s_LogTimestampMode = nsLog::TimestampMode::None;

nsCommandLineOptionPath opt_OrderFile("_TestFramework", "-order", "Path to a file that defines which tests to run.", "");
nsCommandLineOptionPath opt_SettingsFile("_TestFramework", "-settings", "Path to a file containing the test settings.", "");
nsCommandLineOptionBool opt_Run("_TestFramework", "-run", "Makes the tests execute right away.", false);
nsCommandLineOptionBool opt_Close("_TestFramework", "-close", "Makes the application close automatically after the tests are finished.", false);
nsCommandLineOptionBool opt_NoGui("_TestFramework", "-noGui", "Never show a GUI.", false);
nsCommandLineOptionBool opt_HTML("_TestFramework", "-html", "Open summary HTML on error.", false);
nsCommandLineOptionBool opt_Console("_TestFramework", "-console", "Keep the console open.", false);
nsCommandLineOptionBool opt_Timestamps("_TestFramework", "-timestamps", "Show timestamps in logs.", false);
nsCommandLineOptionBool opt_MsgBox("_TestFramework", "-msgbox", "Show message box after tests.", false);
nsCommandLineOptionBool opt_DisableSuccessful("_TestFramework", "-disableSuccessful", "Disable tests that ran successfully.", false);
nsCommandLineOptionBool opt_EnableAllTests("_TestFramework", "-all", "Enable all tests.", false);
nsCommandLineOptionBool opt_NoSave("_TestFramework", "-noSave", "Disables saving of any state.", false);
nsCommandLineOptionInt opt_Revision("_TestFramework", "-rev", "Revision number to pass through to JSON output.", -1);
nsCommandLineOptionInt opt_Passes("_TestFramework", "-passes", "Number of passes to execute.", 1);
nsCommandLineOptionInt opt_Assert("_TestFramework", "-assert", "Whether to assert when a test fails.", (int)AssertOnTestFail::AssertIfDebuggerAttached);
nsCommandLineOptionString opt_Filter("_TestFramework", "-filter", "Filter to execute only certain tests.", "");
nsCommandLineOptionPath opt_Json("_TestFramework", "-json", "JSON file to write.", "");
nsCommandLineOptionPath opt_OutputDir("_TestFramework", "-outputDir", "Output directory", "");

constexpr int s_iMaxErrorMessageLength = 512;

static bool TestAssertHandler(const char* szSourceFile, nsUInt32 uiLine, const char* szFunction, const char* szExpression, const char* szAssertMsg)
{
  if (nsTestFramework::s_bCallstackOnAssert)
  {
    void* pBuffer[64];
    nsArrayPtr<void*> tempTrace(pBuffer);
    const nsUInt32 uiNumTraces = nsStackTracer::GetStackTrace(tempTrace, nullptr);
    nsStackTracer::ResolveStackTrace(tempTrace.GetSubArray(0, uiNumTraces), &nsLog::Print);
  }

  nsTestFramework::Error(szExpression, szSourceFile, (nsInt32)uiLine, szFunction, szAssertMsg);

  // if a debugger is attached, one typically always wants to know about asserts
  if (nsSystemInformation::IsDebuggerAttached())
    return true;

  nsTestFramework::GetInstance()->AbortTests();

  return nsTestFramework::GetAssertOnTestFail();
}

////////////////////////////////////////////////////////////////////////
// nsTestFramework public functions
////////////////////////////////////////////////////////////////////////

nsTestFramework::nsTestFramework(const char* szTestName, const char* szAbsTestOutputDir, const char* szRelTestDataDir, int iArgc, const char** pArgv)
  : m_sTestName(szTestName)
  , m_sAbsTestOutputDir(szAbsTestOutputDir)
  , m_sRelTestDataDir(szRelTestDataDir)
{
  s_pInstance = this;

  nsCommandLineUtils::GetGlobalInstance()->SetCommandLine(iArgc, pArgv, nsCommandLineUtils::PreferOsArgs);

  GetTestSettingsFromCommandLine(*nsCommandLineUtils::GetGlobalInstance());
}

nsTestFramework::~nsTestFramework()
{
  if (m_bIsInitialized)
    DeInitialize();
  s_pInstance = nullptr;
}

void nsTestFramework::Initialize()
{
  {
    nsStringBuilder cmdHelp;
    if (nsCommandLineOption::LogAvailableOptionsToBuffer(cmdHelp, nsCommandLineOption::LogAvailableModes::IfHelpRequested, "_TestFramework;cvar"))
    {
      // make sure the console stays open
      nsCommandLineUtils::GetGlobalInstance()->InjectCustomArgument("-console");
      nsCommandLineUtils::GetGlobalInstance()->InjectCustomArgument("true");

      nsLog::Print(cmdHelp);
    }
  }

  if (m_Settings.m_bNoGUI)
  {
    // if the UI is run with GUI disabled, set the environment variable NS_SILENT_ASSERTS
    // to make sure that no child process that the tests launch shows an assert dialog in case of a crash
#if NS_ENABLED(NS_PLATFORM_WINDOWS_UWP)
    // Not supported
#else
    if (nsEnvironmentVariableUtils::SetValueInt("NS_SILENT_ASSERTS", 1).Failed())
    {
      nsLog::Print("Failed to set 'NS_SILENT_ASSERTS' environment variable!");
    }
#endif
  }

  if (m_Settings.m_bShowTimestampsInLog)
  {
    nsTestFramework::s_LogTimestampMode = nsLog::TimestampMode::TimeOnly;
    nsLogWriter::Console::SetTimestampMode(nsLog::TimestampMode::TimeOnly);
  }

  // Don't do this, it will spam the log with sub-system messages
  // nsGlobalLog::AddLogWriter(nsLogWriter::Console::LogMessageHandler);
  // nsGlobalLog::AddLogWriter(nsLogWriter::VisualStudio::LogMessageHandler);

  nsStartup::AddApplicationTag("testframework");
  nsStartup::StartupCoreSystems();
  NS_SCOPE_EXIT(nsStartup::ShutdownCoreSystems());

  // if tests need to write data back through Fileserve (e.g. image comparison results), they can do that through a data dir mounted with
  // this path
  nsFileSystem::SetSpecialDirectory("nstest", nsTestFramework::GetInstance()->GetAbsOutputPath());

  // Setting ns assert handler
  m_PreviousAssertHandler = nsGetAssertHandler();
  nsSetAssertHandler(TestAssertHandler);

  CreateOutputFolder();
  nsFileSystem::DetectSdkRootDirectory().IgnoreResult();

  nsCommandLineUtils& cmd = *nsCommandLineUtils::GetGlobalInstance();
  // figure out which tests exist
  GatherAllTests();

  if (!m_Settings.m_bNoGUI || opt_OrderFile.IsOptionSpecified(nullptr, &cmd))
  {
    // load the test order from file, if that file does not exist, the array is not modified.
    LoadTestOrder();
  }
  ApplyTestOrderFromCommandLine(cmd);

  if (!m_Settings.m_bNoGUI || opt_SettingsFile.IsOptionSpecified(nullptr, &cmd))
  {
    // Load the test settings from file, if that file does not exist, the settings are not modified.
    LoadTestSettings();
    // Overwrite loaded test settings with command line
    GetTestSettingsFromCommandLine(cmd);
  }

  // save the current order back to the same file
  AutoSaveTestOrder();

  m_bIsInitialized = true;
}

void nsTestFramework::DeInitialize()
{
  m_bIsInitialized = false;

  nsSetAssertHandler(m_PreviousAssertHandler);
  m_PreviousAssertHandler = nullptr;
}

const char* nsTestFramework::GetTestName() const
{
  return m_sTestName.c_str();
}

const char* nsTestFramework::GetAbsOutputPath() const
{
  return m_sAbsTestOutputDir.c_str();
}


const char* nsTestFramework::GetRelTestDataPath() const
{
  return m_sRelTestDataDir.c_str();
}

const char* nsTestFramework::GetAbsTestOrderFilePath() const
{
  return m_sAbsTestOrderFilePath.c_str();
}

const char* nsTestFramework::GetAbsTestSettingsFilePath() const
{
  return m_sAbsTestSettingsFilePath.c_str();
}

void nsTestFramework::RegisterOutputHandler(OutputHandler handler)
{
  // do not register a handler twice
  for (nsUInt32 i = 0; i < m_OutputHandlers.size(); ++i)
  {
    if (m_OutputHandlers[i] == handler)
      return;
  }

  m_OutputHandlers.push_back(handler);
}


void nsTestFramework::SetImageDiffExtraInfoCallback(ImageDiffExtraInfoCallback provider)
{
  m_ImageDiffExtraInfoCallback = provider;
}

bool nsTestFramework::GetAssertOnTestFail()
{
  switch (s_pInstance->m_Settings.m_AssertOnTestFail)
  {
    case AssertOnTestFail::DoNotAssert:
      return false;
    case AssertOnTestFail::AssertIfDebuggerAttached:
      return nsSystemInformation::IsDebuggerAttached();
    case AssertOnTestFail::AlwaysAssert:
      return true;
  }
  return false;
}

void nsTestFramework::GatherAllTests()
{
  m_TestEntries.clear();

  m_iErrorCount = 0;
  m_iTestsFailed = 0;
  m_iTestsPassed = 0;
  m_uiExecutingTest = nsInvalidIndex;
  m_uiExecutingSubTest = nsInvalidIndex;
  m_bSubTestInitialized = false;

  // first let all simple tests register themselves
  {
    nsRegisterSimpleTestHelper* pHelper = nsRegisterSimpleTestHelper::GetFirstInstance();

    while (pHelper)
    {
      pHelper->RegisterTest();

      pHelper = pHelper->GetNextInstance();
    }
  }

  nsTestConfiguration config;
  nsTestBaseClass* pTestClass = nsTestBaseClass::GetFirstInstance();

  while (pTestClass)
  {
    pTestClass->ClearSubTests();
    pTestClass->SetupSubTests();
    pTestClass->UpdateConfiguration(config);

    nsTestEntry e;
    e.m_pTest = pTestClass;
    e.m_szTestName = pTestClass->GetTestName();
    e.m_sNotAvailableReason = pTestClass->IsTestAvailable();

    for (nsUInt32 i = 0; i < pTestClass->m_Entries.size(); ++i)
    {
      nsSubTestEntry st;
      st.m_szSubTestName = pTestClass->m_Entries[i].m_szName;
      st.m_iSubTestIdentifier = pTestClass->m_Entries[i].m_iIdentifier;

      e.m_SubTests.push_back(st);
    }

    m_TestEntries.push_back(e);

    pTestClass = pTestClass->GetNextInstance();
  }
  ::SortTestsAlphabetically(m_TestEntries);

  m_Result.SetupTests(m_TestEntries, config);
}

void nsTestFramework::GetTestSettingsFromCommandLine(const nsCommandLineUtils& cmd)
{
  // use a local instance of nsCommandLineUtils as global instance is not guaranteed to have been set up
  // for all call sites of this method.

  m_Settings.m_bRunTests = opt_Run.GetOptionValue(nsCommandLineOption::LogMode::AlwaysIfSpecified, &cmd);
  m_Settings.m_bCloseOnSuccess = opt_Close.GetOptionValue(nsCommandLineOption::LogMode::AlwaysIfSpecified, &cmd);
  m_Settings.m_bNoGUI = opt_NoGui.GetOptionValue(nsCommandLineOption::LogMode::AlwaysIfSpecified, &cmd);

  if (opt_Assert.IsOptionSpecified(nullptr, &cmd))
  {
    const int assertOnTestFailure = opt_Assert.GetOptionValue(nsCommandLineOption::LogMode::AlwaysIfSpecified, &cmd);
    switch (assertOnTestFailure)
    {
      case 0:
        m_Settings.m_AssertOnTestFail = AssertOnTestFail::DoNotAssert;
        break;
      case 1:
        m_Settings.m_AssertOnTestFail = AssertOnTestFail::AssertIfDebuggerAttached;
        break;
      case 2:
        m_Settings.m_AssertOnTestFail = AssertOnTestFail::AlwaysAssert;
        break;
    }
  }

  nsStringBuilder tmp;

  opt_HTML.SetDefaultValue(m_Settings.m_bOpenHtmlOutputOnError);
  m_Settings.m_bOpenHtmlOutputOnError = opt_HTML.GetOptionValue(nsCommandLineOption::LogMode::AlwaysIfSpecified, &cmd);

  opt_Console.SetDefaultValue(m_Settings.m_bKeepConsoleOpen);
  m_Settings.m_bKeepConsoleOpen = opt_Console.GetOptionValue(nsCommandLineOption::LogMode::AlwaysIfSpecified, &cmd);

  opt_Timestamps.SetDefaultValue(m_Settings.m_bShowTimestampsInLog);
  m_Settings.m_bShowTimestampsInLog = opt_Timestamps.GetOptionValue(nsCommandLineOption::LogMode::AlwaysIfSpecified, &cmd);

  opt_MsgBox.SetDefaultValue(m_Settings.m_bShowMessageBox);
  m_Settings.m_bShowMessageBox = opt_MsgBox.GetOptionValue(nsCommandLineOption::LogMode::AlwaysIfSpecified, &cmd);

  opt_DisableSuccessful.SetDefaultValue(m_Settings.m_bAutoDisableSuccessfulTests);
  m_Settings.m_bAutoDisableSuccessfulTests = opt_DisableSuccessful.GetOptionValue(nsCommandLineOption::LogMode::AlwaysIfSpecified, &cmd);

  m_Settings.m_iRevision = opt_Revision.GetOptionValue(nsCommandLineOption::LogMode::AlwaysIfSpecified, &cmd);
  m_Settings.m_bEnableAllTests = opt_EnableAllTests.GetOptionValue(nsCommandLineOption::LogMode::AlwaysIfSpecified, &cmd);
  m_Settings.m_uiFullPasses = static_cast<nsUInt8>(opt_Passes.GetOptionValue(nsCommandLineOption::LogMode::AlwaysIfSpecified, &cmd));
  m_Settings.m_sTestFilter = opt_Filter.GetOptionValue(nsCommandLineOption::LogMode::AlwaysIfSpecified, &cmd).GetData(tmp);

  if (opt_Json.IsOptionSpecified(nullptr, &cmd))
  {
    m_Settings.m_sJsonOutput = opt_Json.GetOptionValue(nsCommandLineOption::LogMode::AlwaysIfSpecified, &cmd);
  }

  if (opt_OutputDir.IsOptionSpecified(nullptr, &cmd))
  {
    m_sAbsTestOutputDir = opt_OutputDir.GetOptionValue(nsCommandLineOption::LogMode::AlwaysIfSpecified, &cmd);
  }

  bool bNoAutoSave = false;
  if (opt_OrderFile.IsOptionSpecified(nullptr, &cmd))
  {
    m_sAbsTestOrderFilePath = opt_OrderFile.GetOptionValue(nsCommandLineOption::LogMode::Always);
    // If a custom order file was provided, default to -nosave as to not overwrite that file with additional
    // parameters from command line. Use "-nosave false" to explicitly enable auto save in this case.
    bNoAutoSave = true;
  }
  else
  {
    m_sAbsTestOrderFilePath = m_sAbsTestOutputDir + std::string("/TestOrder.txt");
  }

  if (opt_SettingsFile.IsOptionSpecified(nullptr, &cmd))
  {
    m_sAbsTestSettingsFilePath = opt_SettingsFile.GetOptionValue(nsCommandLineOption::LogMode::Always);
    // If a custom settings file was provided, default to -nosave as to not overwrite that file with additional
    // parameters from command line. Use "-nosave false" to explicitly enable auto save in this case.
    bNoAutoSave = true;
  }
  else
  {
    m_sAbsTestSettingsFilePath = m_sAbsTestOutputDir + std::string("/TestSettings.txt");
  }
  opt_NoSave.SetDefaultValue(bNoAutoSave);
  m_Settings.m_bNoAutomaticSaving = opt_NoSave.GetOptionValue(nsCommandLineOption::LogMode::AlwaysIfSpecified, &cmd);

  m_uiPassesLeft = m_Settings.m_uiFullPasses;
}

void nsTestFramework::LoadTestOrder()
{
  ::LoadTestOrder(m_sAbsTestOrderFilePath.c_str(), m_TestEntries);
}

void nsTestFramework::ApplyTestOrderFromCommandLine(const nsCommandLineUtils& cmd)
{
  if (m_Settings.m_bEnableAllTests)
    SetAllTestsEnabledStatus(true);
  if (!m_Settings.m_sTestFilter.empty())
  {
    const nsUInt32 uiTestCount = GetTestCount();
    for (nsUInt32 uiTestIdx = 0; uiTestIdx < uiTestCount; ++uiTestIdx)
    {
      const bool bEnable = nsStringUtils::FindSubString_NoCase(m_TestEntries[uiTestIdx].m_szTestName, m_Settings.m_sTestFilter.c_str()) != nullptr;
      m_TestEntries[uiTestIdx].m_bEnableTest = bEnable;
      const nsUInt32 uiSubTestCount = (nsUInt32)m_TestEntries[uiTestIdx].m_SubTests.size();
      for (nsUInt32 uiSubTest = 0; uiSubTest < uiSubTestCount; ++uiSubTest)
      {
        m_TestEntries[uiTestIdx].m_SubTests[uiSubTest].m_bEnableTest = bEnable;
      }
    }
  }
}

void nsTestFramework::LoadTestSettings()
{
  ::LoadTestSettings(m_sAbsTestSettingsFilePath.c_str(), m_Settings);
}

void nsTestFramework::CreateOutputFolder()
{
  nsOSFile::CreateDirectoryStructure(m_sAbsTestOutputDir.c_str()).IgnoreResult();

  NS_ASSERT_RELEASE(nsOSFile::ExistsDirectory(m_sAbsTestOutputDir.c_str()), "Failed to create output directory '{0}'", m_sAbsTestOutputDir.c_str());
}

void nsTestFramework::UpdateReferenceImages()
{
  nsStringBuilder sDir;
  if (nsFileSystem::ResolveSpecialDirectory(">sdk", sDir).Failed())
    return;

  sDir.AppendPath(GetRelTestDataPath());

  const nsStringBuilder sNewFiles(m_sAbsTestOutputDir.c_str(), "/Images_Result");
  const nsStringBuilder sRefFiles(sDir, "/", m_sImageReferenceFolderName.c_str());

#if NS_ENABLED(NS_SUPPORTS_FILE_ITERATORS) && NS_ENABLED(NS_SUPPORTS_FILE_STATS)


#  if NS_ENABLED(NS_PLATFORM_WINDOWS_DESKTOP)
  nsStringBuilder sOptiPng = nsFileSystem::GetSdkRootDirectory();
  sOptiPng.AppendPath("Data/Tools/Precompiled/optipng/optipng.exe");

  if (nsOSFile::ExistsFile(sOptiPng))
  {
    nsStringBuilder sPath;

    nsFileSystemIterator it;
    it.StartSearch(sNewFiles, nsFileSystemIteratorFlags::ReportFiles);
    for (; it.IsValid(); it.Next())
    {
      it.GetStats().GetFullPath(sPath);

      nsProcessOptions opt;
      opt.m_sProcess = sOptiPng;
      opt.m_Arguments.PushBack(sPath);
      nsProcess::Execute(opt).IgnoreResult();
    }
  }

#  endif

  // if some target files already exist somewhere (ie. custom folders for the tests)
  // overwrite the existing files in their location
  {
    nsHybridArray<nsString, 32> targetFolders;
    nsStringBuilder sFullPath, sTargetPath;

    {
      nsFileSystemIterator it;
      it.StartSearch(sDir, nsFileSystemIteratorFlags::ReportFoldersRecursive);
      for (; it.IsValid(); it.Next())
      {
        if (it.GetStats().m_sName == m_sImageReferenceFolderName.c_str())
        {
          it.GetStats().GetFullPath(sFullPath);

          targetFolders.PushBack(sFullPath);
        }
      }
    }

    nsFileSystemIterator it;
    it.StartSearch(sNewFiles, nsFileSystemIteratorFlags::ReportFiles);
    for (; it.IsValid(); it.Next())
    {
      it.GetStats().GetFullPath(sFullPath);

      for (nsUInt32 i = 0; i < targetFolders.GetCount(); ++i)
      {
        sTargetPath = targetFolders[i];
        sTargetPath.AppendPath(it.GetStats().m_sName);

        if (nsOSFile::ExistsFile(sTargetPath))
        {
          nsOSFile::DeleteFile(sTargetPath).IgnoreResult();
          nsOSFile::MoveFileOrDirectory(sFullPath, sTargetPath).IgnoreResult();
          break;
        }
      }
    }
  }

  // copy the remaining files to the default directory
  nsOSFile::CopyFolder(sNewFiles, sRefFiles).IgnoreResult();
  nsOSFile::DeleteFolder(sNewFiles).IgnoreResult();
#endif
}

void nsTestFramework::AutoSaveTestOrder()
{
  if (m_Settings.m_bNoAutomaticSaving)
    return;

  SaveTestOrder(m_sAbsTestOrderFilePath.c_str());
  SaveTestSettings(m_sAbsTestSettingsFilePath.c_str());
}

void nsTestFramework::SaveTestOrder(const char* const szFilePath)
{
  ::SaveTestOrder(szFilePath, m_TestEntries);
}

void nsTestFramework::SaveTestSettings(const char* const szFilePath)
{
  ::SaveTestSettings(szFilePath, m_Settings);
}

void nsTestFramework::SetAllTestsEnabledStatus(bool bEnable)
{
  const nsUInt32 uiTestCount = GetTestCount();
  for (nsUInt32 uiTestIdx = 0; uiTestIdx < uiTestCount; ++uiTestIdx)
  {
    m_TestEntries[uiTestIdx].m_bEnableTest = bEnable;
    const nsUInt32 uiSubTestCount = (nsUInt32)m_TestEntries[uiTestIdx].m_SubTests.size();
    for (nsUInt32 uiSubTest = 0; uiSubTest < uiSubTestCount; ++uiSubTest)
    {
      m_TestEntries[uiTestIdx].m_SubTests[uiSubTest].m_bEnableTest = bEnable;
    }
  }
}

void nsTestFramework::SetAllFailedTestsEnabledStatus()
{
  const auto& LastResult = GetTestResult();

  const nsUInt32 uiTestCount = GetTestCount();
  for (nsUInt32 uiTestIdx = 0; uiTestIdx < uiTestCount; ++uiTestIdx)
  {
    const auto& TestRes = LastResult.GetTestResultData(uiTestIdx, -1);
    m_TestEntries[uiTestIdx].m_bEnableTest = TestRes.m_bExecuted && !TestRes.m_bSuccess;

    const nsUInt32 uiSubTestCount = (nsUInt32)m_TestEntries[uiTestIdx].m_SubTests.size();
    for (nsUInt32 uiSubTest = 0; uiSubTest < uiSubTestCount; ++uiSubTest)
    {
      const auto& SubTestRes = LastResult.GetTestResultData(uiTestIdx, uiSubTest);
      m_TestEntries[uiTestIdx].m_SubTests[uiSubTest].m_bEnableTest = SubTestRes.m_bExecuted && !SubTestRes.m_bSuccess;
    }
  }
}

void nsTestFramework::SetTestTimeout(nsUInt32 uiTestTimeoutMS)
{
  {
    std::scoped_lock<std::mutex> lock(m_TimeoutLock);
    m_uiTimeoutMS = uiTestTimeoutMS;
  }
  UpdateTestTimeout();
}

nsUInt32 nsTestFramework::GetTestTimeout() const
{
  return m_uiTimeoutMS;
}

void nsTestFramework::TimeoutThread()
{
  std::unique_lock<std::mutex> lock(m_TimeoutLock);
  while (m_bUseTimeout)
  {
    if (m_uiTimeoutMS == 0)
    {
      // If no timeout is set, we simply put the thread to sleep.
      m_TimeoutCV.wait(lock, [this]
        { return !m_bUseTimeout; });
    }
    // We want to be notified when we reach the timeout and not when we are spuriously woken up.
    // Thus we continue waiting via the predicate if we are still using a timeout until we are either
    // woken up via the CV or reach the timeout.
    else if (!m_TimeoutCV.wait_for(lock, std::chrono::milliseconds(m_uiTimeoutMS), [this]
               { return !m_bUseTimeout || m_bArm; }))
    {
      if (nsSystemInformation::IsDebuggerAttached())
      {
        // Should we attach a debugger mid run and reach the timeout we obviously do not want to terminate.
        continue;
      }

      // CV was not signaled until the timeout was reached.
      nsTestFramework::Output(nsTestOutput::Error, "Timeout reached, terminating app.");
      // The top level exception handler takes care of all the shutdown logic already (app specific logic, crash dump, callstack etc)
      // which we do not want to duplicate here so we simply throw an unhandled exception.
      throw std::runtime_error("Timeout reached, terminating app.");
    }
    m_bArm = false;
  }
}


void nsTestFramework::UpdateTestTimeout()
{
  {
    std::scoped_lock<std::mutex> lock(m_TimeoutLock);
    if (!m_bUseTimeout)
    {
      return;
    }
    m_bArm = true;
  }
  m_TimeoutCV.notify_one();
}

void nsTestFramework::ResetTests()
{
  m_iErrorCount = 0;
  m_iTestsFailed = 0;
  m_iTestsPassed = 0;
  m_uiExecutingTest = nsInvalidIndex;
  m_uiExecutingSubTest = nsInvalidIndex;
  m_bSubTestInitialized = false;
  m_bAbortTests = false;

  m_Result.Reset();
}

nsTestAppRun nsTestFramework::RunTestExecutionLoop()
{
  if (!m_bIsInitialized)
  {
    Initialize();

#ifdef NS_TESTFRAMEWORK_USE_FILESERVE
    if (nsFileserveClient::GetSingleton() == nullptr)
    {
      NS_DEFAULT_NEW(nsFileserveClient);

      if (nsFileserveClient::GetSingleton()->SearchForServerAddress().Failed())
      {
        nsFileserveClient::GetSingleton()->WaitForServerInfo().IgnoreResult();
      }
    }

    if (nsFileserveClient::GetSingleton()->EnsureConnected(nsTime::MakeFromSeconds(-30)).Failed())
    {
      Error("Failed to establish a Fileserve connection", "", 0, "nsTestFramework::RunTestExecutionLoop", "");
      return nsTestAppRun::Quit;
    }
#endif
  }

#ifdef NS_TESTFRAMEWORK_USE_FILESERVE
  nsFileserveClient::GetSingleton()->UpdateClient();
#endif


  if (m_uiExecutingTest == nsInvalidIndex)
  {
    StartTests();
    m_uiExecutingTest = 0;
    NS_ASSERT_DEV(m_uiExecutingSubTest == nsInvalidIndex, "Invalid test framework state");
    NS_ASSERT_DEV(!m_bSubTestInitialized, "Invalid test framework state");
  }

  ExecuteNextTest();

  if (m_uiExecutingTest >= (nsUInt32)m_TestEntries.size())
  {
    EndTests();

    if (m_uiPassesLeft > 1 && !m_bAbortTests)
    {
      --m_uiPassesLeft;

      m_uiExecutingTest = nsInvalidIndex;
      m_uiExecutingSubTest = nsInvalidIndex;

      return nsTestAppRun::Continue;
    }

#ifdef NS_TESTFRAMEWORK_USE_FILESERVE
    if (nsFileserveClient* pClient = nsFileserveClient::GetSingleton())
    {
      // shutdown the fileserve client
      NS_DEFAULT_DELETE(pClient);
    }
#endif

    return nsTestAppRun::Quit;
  }

  return nsTestAppRun::Continue;
}

void nsTestFramework::StartTests()
{
  ResetTests();
  m_bTestsRunning = true;
  nsTestFramework::Output(nsTestOutput::StartOutput, "");

  // Start timeout thread.
  std::scoped_lock lock(m_TimeoutLock);
  m_bUseTimeout = true;
  m_bArm = false;
  m_TimeoutThread = std::thread(&nsTestFramework::TimeoutThread, this);
}

// Redirects engine warnings / errors to test-framework output
static void LogWriter(const nsLoggingEventData& e)
{
  const nsStringBuilder sText = e.m_sText;

  switch (e.m_EventType)
  {
    case nsLogMsgType::ErrorMsg:
      nsTestFramework::Output(nsTestOutput::Error, "nsLog Error: %s", sText.GetData());
      break;
    case nsLogMsgType::SeriousWarningMsg:
      nsTestFramework::Output(nsTestOutput::Error, "nsLog Serious Warning: %s", sText.GetData());
      break;
    case nsLogMsgType::WarningMsg:
      nsTestFramework::Output(nsTestOutput::Warning, "nsLog Warning: %s", sText.GetData());
      break;
    case nsLogMsgType::InfoMsg:
    case nsLogMsgType::DevMsg:
    case nsLogMsgType::DebugMsg:
    {
      if (e.m_sTag.IsEqual_NoCase("test"))
        nsTestFramework::Output(nsTestOutput::Details, sText.GetData());
    }
    break;

    default:
      return;
  }
}

void nsTestFramework::ExecuteNextTest()
{
  NS_ASSERT_DEV(m_uiExecutingTest >= 0, "Invalid current test.");

  if (m_uiExecutingTest == (nsUInt32)GetTestCount())
    return;

  if (!m_TestEntries[m_uiExecutingTest].m_bEnableTest)
  {
    // next time run the next test and start with the first subtest
    m_uiExecutingTest++;
    m_uiExecutingSubTest = nsInvalidIndex;
    return;
  }

  nsTestEntry& TestEntry = m_TestEntries[m_uiExecutingTest];
  nsTestBaseClass* pTestClass = m_TestEntries[m_uiExecutingTest].m_pTest;

  // Execute test
  {
    if (m_uiExecutingSubTest == nsInvalidIndex) // no subtest has run yet, so initialize the test first
    {
      if (m_bAbortTests)
      {
        m_uiExecutingTest = (nsUInt32)m_TestEntries.size(); // skip to the end of all tests
        m_uiExecutingSubTest = nsInvalidIndex;
        return;
      }

      m_uiExecutingSubTest = 0;
      m_fTotalTestDuration = 0.0;

      // Reset assert counter. This variable is used to reduce the overhead of counting millions of asserts.
      s_iAssertCounter = 0;
      m_uiCurrentTestIndex = m_uiExecutingTest;
      // Log writer translates engine warnings / errors into test framework error messages.
      nsGlobalLog::AddLogWriter(LogWriter);

      m_iErrorCountBeforeTest = GetTotalErrorCount();

      nsTestFramework::Output(nsTestOutput::BeginBlock, "Executing Test: '%s'", TestEntry.m_szTestName);

      // *** Test Initialization ***
      if (TestEntry.m_sNotAvailableReason.empty())
      {
        UpdateTestTimeout();
        if (pTestClass->DoTestInitialization().Failed())
        {
          m_uiExecutingSubTest = (nsUInt32)TestEntry.m_SubTests.size(); // make sure all sub-tests are skipped
        }
      }
      else
      {
        nsTestFramework::Output(nsTestOutput::ImportantInfo, "Test not available: %s", TestEntry.m_sNotAvailableReason.c_str());
        m_uiExecutingSubTest = (nsUInt32)TestEntry.m_SubTests.size(); // make sure all sub-tests are skipped
      }
    }

    if (m_uiExecutingSubTest < (nsUInt32)TestEntry.m_SubTests.size())
    {
      nsSubTestEntry& subTest = TestEntry.m_SubTests[m_uiExecutingSubTest];
      nsInt32 iSubTestIdentifier = subTest.m_iSubTestIdentifier;

      if (!subTest.m_bEnableTest)
      {
        ++m_uiExecutingSubTest;
        return;
      }

      if (!m_bSubTestInitialized)
      {
        if (m_bAbortTests)
        {
          // tests shall be aborted, so do not start a new one

          m_uiExecutingTest = (nsInt32)m_TestEntries.size(); // skip to the end of all tests
          m_uiExecutingSubTest = nsInvalidIndex;
          return;
        }

        m_fTotalSubTestDuration = 0.0;
        m_uiSubTestInvocationCount = 0;

        // First flush of assert counter, these are all asserts during test init.
        FlushAsserts();
        m_uiCurrentSubTestIndex = m_uiExecutingSubTest;
        nsTestFramework::Output(nsTestOutput::BeginBlock, "Executing Sub-Test: '%s'", subTest.m_szSubTestName);

        // *** Sub-Test Initialization ***
        UpdateTestTimeout();
        m_bSubTestInitialized = pTestClass->DoSubTestInitialization(iSubTestIdentifier).Succeeded();
      }

      nsTestAppRun subTestResult = nsTestAppRun::Quit;

      if (m_bSubTestInitialized)
      {
        // *** Run Sub-Test ***
        double fDuration = 0.0;

        // start with 1
        ++m_uiSubTestInvocationCount;

        UpdateTestTimeout();
        subTestResult = pTestClass->DoSubTestRun(iSubTestIdentifier, fDuration, m_uiSubTestInvocationCount);
        s_szTestBlockName = "";

        if (m_bImageComparisonScheduled)
        {
          NS_TEST_IMAGE(m_uiComparisonImageNumber, m_uiMaxImageComparisonError);
          m_bImageComparisonScheduled = false;
        }


        if (m_bDepthImageComparisonScheduled)
        {
          NS_TEST_DEPTH_IMAGE(m_uiComparisonDepthImageNumber, m_uiMaxDepthImageComparisonError);
          m_bDepthImageComparisonScheduled = false;
        }

        // I guess we can require that tests are written in a way that they can be interrupted
        if (m_bAbortTests)
          subTestResult = nsTestAppRun::Quit;

        m_fTotalSubTestDuration += fDuration;
      }

      // this is executed when sub-test initialization failed or the sub-test reached its end
      if (subTestResult == nsTestAppRun::Quit)
      {
        // *** Sub-Test De-Initialization ***
        UpdateTestTimeout();
        pTestClass->DoSubTestDeInitialization(iSubTestIdentifier);

        bool bSubTestSuccess = m_bSubTestInitialized && (m_Result.GetErrorMessageCount(m_uiExecutingTest, m_uiExecutingSubTest) == 0);
        nsTestFramework::TestResult(m_uiExecutingSubTest, bSubTestSuccess, m_fTotalSubTestDuration);

        m_fTotalTestDuration += m_fTotalSubTestDuration;

        // advance to the next (sub) test
        m_bSubTestInitialized = false;
        ++m_uiExecutingSubTest;

        // Second flush of assert counter, these are all asserts for the current subtest.
        FlushAsserts();
        nsTestFramework::Output(nsTestOutput::EndBlock, "");
        m_uiCurrentSubTestIndex = nsInvalidIndex;
      }
    }

    if (m_bAbortTests || m_uiExecutingSubTest >= (nsInt32)TestEntry.m_SubTests.size())
    {
      // *** Test De-Initialization ***
      if (TestEntry.m_sNotAvailableReason.empty())
      {
        // We only call DoTestInitialization under this condition so DoTestDeInitialization must be guarded by the same.
        UpdateTestTimeout();
        pTestClass->DoTestDeInitialization();
      }
      // Third and last flush of assert counter, these are all asserts for the test de-init.
      FlushAsserts();

      nsGlobalLog::RemoveLogWriter(LogWriter);

      bool bTestSuccess = m_iErrorCountBeforeTest == GetTotalErrorCount();
      nsTestFramework::TestResult(-1, bTestSuccess, m_fTotalTestDuration);
      nsTestFramework::Output(nsTestOutput::EndBlock, "");
      m_uiCurrentTestIndex = nsInvalidIndex;

      // advance to the next test
      m_uiExecutingTest++;
      m_uiExecutingSubTest = nsInvalidIndex;
    }
  }
}

void nsTestFramework::EndTests()
{
  m_bTestsRunning = false;
  if (GetTestsFailedCount() == 0)
    nsTestFramework::Output(nsTestOutput::FinalResult, "All tests passed.");
  else
    nsTestFramework::Output(nsTestOutput::FinalResult, "Tests failed: %i. Tests passed: %i", GetTestsFailedCount(), GetTestsPassedCount());

  if (!m_Settings.m_sJsonOutput.empty())
    m_Result.WriteJsonToFile(m_Settings.m_sJsonOutput.c_str());

  m_uiExecutingTest = nsInvalidIndex;
  m_uiExecutingSubTest = nsInvalidIndex;
  m_bAbortTests = false;

  // Stop timeout thread.
  {
    std::scoped_lock lock(m_TimeoutLock);
    m_bUseTimeout = false;
    m_TimeoutCV.notify_one();
  }
  m_TimeoutThread.join();
}

void nsTestFramework::AbortTests()
{
  m_bAbortTests = true;
}

nsUInt32 nsTestFramework::GetTestCount() const
{
  return (nsUInt32)m_TestEntries.size();
}

nsUInt32 nsTestFramework::GetTestEnabledCount() const
{
  nsUInt32 uiEnabledCount = 0;
  const nsUInt32 uiTests = GetTestCount();
  for (nsUInt32 uiTest = 0; uiTest < uiTests; ++uiTest)
  {
    uiEnabledCount += m_TestEntries[uiTest].m_bEnableTest ? 1 : 0;
  }
  return uiEnabledCount;
}

nsUInt32 nsTestFramework::GetSubTestEnabledCount(nsUInt32 uiTestIndex) const
{
  if (uiTestIndex >= GetTestCount())
    return 0;

  nsUInt32 uiEnabledCount = 0;
  const nsUInt32 uiSubTests = (nsUInt32)m_TestEntries[uiTestIndex].m_SubTests.size();
  for (nsUInt32 uiSubTest = 0; uiSubTest < uiSubTests; ++uiSubTest)
  {
    uiEnabledCount += m_TestEntries[uiTestIndex].m_SubTests[uiSubTest].m_bEnableTest ? 1 : 0;
  }
  return uiEnabledCount;
}

const std::string& nsTestFramework::IsTestAvailable(nsUInt32 uiTestIndex) const
{
  NS_ASSERT_DEV(uiTestIndex < GetTestCount(), "Test index {0} is larger than number of tests {1}.", uiTestIndex, GetTestCount());
  return m_TestEntries[uiTestIndex].m_sNotAvailableReason;
}

bool nsTestFramework::IsTestEnabled(nsUInt32 uiTestIndex) const
{
  if (uiTestIndex >= GetTestCount())
    return false;

  return m_TestEntries[uiTestIndex].m_bEnableTest;
}

bool nsTestFramework::IsSubTestEnabled(nsUInt32 uiTestIndex, nsUInt32 uiSubTestIndex) const
{
  if (uiTestIndex >= GetTestCount())
    return false;

  const nsUInt32 uiSubTests = (nsUInt32)m_TestEntries[uiTestIndex].m_SubTests.size();
  if (uiSubTestIndex >= uiSubTests)
    return false;

  return m_TestEntries[uiTestIndex].m_SubTests[uiSubTestIndex].m_bEnableTest;
}

void nsTestFramework::SetTestEnabled(nsUInt32 uiTestIndex, bool bEnabled)
{
  if (uiTestIndex >= GetTestCount())
    return;

  m_TestEntries[uiTestIndex].m_bEnableTest = bEnabled;
}

void nsTestFramework::SetSubTestEnabled(nsUInt32 uiTestIndex, nsUInt32 uiSubTestIndex, bool bEnabled)
{
  if (uiTestIndex >= GetTestCount())
    return;

  const nsUInt32 uiSubTests = (nsUInt32)m_TestEntries[uiTestIndex].m_SubTests.size();
  if (uiSubTestIndex >= uiSubTests)
    return;

  m_TestEntries[uiTestIndex].m_SubTests[uiSubTestIndex].m_bEnableTest = bEnabled;
}

nsInt32 nsTestFramework::GetCurrentSubTestIdentifier() const
{
  return GetCurrentSubTest()->m_iSubTestIdentifier;
}

nsUInt32 nsTestFramework::FindSubTestIndexForSubTestIdentifier(nsInt32 iSubTestIdentifier) const
{
  const nsTestEntry* pTest = GetCurrentTest();

  const nsUInt32 uiSubTests = (nsUInt32)pTest->m_SubTests.size();
  for (nsUInt32 i = 0; i < uiSubTests; ++i)
  {
    if (pTest->m_SubTests[i].m_iSubTestIdentifier == iSubTestIdentifier)
      return i;
  }

  return nsInvalidIndex;
}

nsTestEntry* nsTestFramework::GetTest(nsUInt32 uiTestIndex)
{
  if (uiTestIndex >= GetTestCount())
    return nullptr;

  return &m_TestEntries[uiTestIndex];
}

const nsTestEntry* nsTestFramework::GetTest(nsUInt32 uiTestIndex) const
{
  if (uiTestIndex >= GetTestCount())
    return nullptr;

  return &m_TestEntries[uiTestIndex];
}

const nsTestEntry* nsTestFramework::GetCurrentTest() const
{
  return GetTest(GetCurrentTestIndex());
}

const nsSubTestEntry* nsTestFramework::GetCurrentSubTest() const
{
  if (auto pTest = GetCurrentTest())
  {
    if (m_uiCurrentSubTestIndex >= (nsInt32)pTest->m_SubTests.size())
      return nullptr;

    return &pTest->m_SubTests[m_uiCurrentSubTestIndex];
  }

  return nullptr;
}

TestSettings nsTestFramework::GetSettings() const
{
  return m_Settings;
}

void nsTestFramework::SetSettings(const TestSettings& settings)
{
  m_Settings = settings;
}

nsTestFrameworkResult& nsTestFramework::GetTestResult()
{
  return m_Result;
}

nsInt32 nsTestFramework::GetTotalErrorCount() const
{
  return m_iErrorCount;
}

nsInt32 nsTestFramework::GetTestsPassedCount() const
{
  return m_iTestsPassed;
}

nsInt32 nsTestFramework::GetTestsFailedCount() const
{
  return m_iTestsFailed;
}

double nsTestFramework::GetTotalTestDuration() const
{
  return m_Result.GetTotalTestDuration();
}

////////////////////////////////////////////////////////////////////////
// nsTestFramework protected functions
////////////////////////////////////////////////////////////////////////

static bool g_bBlockOutput = false;

void nsTestFramework::OutputImpl(nsTestOutput::Enum Type, const char* szMsg)
{
  std::scoped_lock _(m_OutputMutex);

  if (Type == nsTestOutput::Error)
  {
    m_iErrorCount++;
  }
  // pass the output to all the registered output handlers, which will then write it to the console, file, etc.
  for (nsUInt32 i = 0; i < m_OutputHandlers.size(); ++i)
  {
    m_OutputHandlers[i](Type, szMsg);
  }

  if (g_bBlockOutput)
    return;

  m_Result.TestOutput(m_uiCurrentTestIndex, m_uiCurrentSubTestIndex, Type, szMsg);
}

void nsTestFramework::ErrorImpl(const char* szError, const char* szFile, nsInt32 iLine, const char* szFunction, const char* szMsg)
{
  std::scoped_lock _(m_OutputMutex);

  m_Result.TestError(m_uiCurrentTestIndex, m_uiCurrentSubTestIndex, szError, nsTestFramework::s_szTestBlockName, szFile, iLine, szFunction, szMsg);

  g_bBlockOutput = true;
  nsTestFramework::Output(nsTestOutput::Error, "%s", szError); // This will also increase the global error count.
  nsTestFramework::Output(nsTestOutput::BeginBlock, "");
  {
    if ((nsTestFramework::s_szTestBlockName != nullptr) && (nsTestFramework::s_szTestBlockName[0] != '\0'))
      nsTestFramework::Output(nsTestOutput::Message, "Block: '%s'", nsTestFramework::s_szTestBlockName);

    nsTestFramework::Output(nsTestOutput::ImportantInfo, "File: %s", szFile);
    nsTestFramework::Output(nsTestOutput::ImportantInfo, "Line: %i", iLine);
    nsTestFramework::Output(nsTestOutput::ImportantInfo, "Function: %s", szFunction);

    if ((szMsg != nullptr) && (szMsg[0] != '\0'))
      nsTestFramework::Output(nsTestOutput::Message, "Error: %s", szMsg);
  }
  nsTestFramework::Output(nsTestOutput::EndBlock, "");
  g_bBlockOutput = false;
}

void nsTestFramework::TestResultImpl(nsUInt32 uiSubTestIndex, bool bSuccess, double fDuration)
{
  std::scoped_lock _(m_OutputMutex);

  m_Result.TestResult(m_uiCurrentTestIndex, uiSubTestIndex, bSuccess, fDuration);

  const nsUInt32 uiMin = (nsUInt32)(fDuration / 1000.0 / 60.0);
  const nsUInt32 uiSec = (nsUInt32)(fDuration / 1000.0 - uiMin * 60.0);
  const nsUInt32 uiMS = (nsUInt32)(fDuration - uiSec * 1000.0);

  nsTestFramework::Output(nsTestOutput::Duration, "%i:%02i:%03i", uiMin, uiSec, uiMS);

  if (uiSubTestIndex == nsInvalidIndex)
  {
    const char* szTestName = m_TestEntries[m_uiCurrentTestIndex].m_szTestName;
    if (bSuccess)
    {
      m_iTestsPassed++;
      nsTestFramework::Output(nsTestOutput::Success, "Test '%s' succeeded (%.2f sec).", szTestName, m_fTotalTestDuration / 1000.0f);

      if (GetSettings().m_bAutoDisableSuccessfulTests)
      {
        m_TestEntries[m_uiCurrentTestIndex].m_bEnableTest = false;
        nsTestFramework::AutoSaveTestOrder();
      }
    }
    else
    {
      m_iTestsFailed++;
      nsTestFramework::Output(nsTestOutput::Error, "Test '%s' failed: %i Errors (%.2f sec).", szTestName, (nsUInt32)m_Result.GetErrorMessageCount(m_uiCurrentTestIndex, uiSubTestIndex), m_fTotalTestDuration / 1000.0f);
    }
  }
  else
  {
    const char* szSubTestName = m_TestEntries[m_uiCurrentTestIndex].m_SubTests[uiSubTestIndex].m_szSubTestName;
    if (bSuccess)
    {
      nsTestFramework::Output(nsTestOutput::Success, "Sub-Test '%s' succeeded (%.2f sec).", szSubTestName, m_fTotalSubTestDuration / 1000.0f);

      if (GetSettings().m_bAutoDisableSuccessfulTests)
      {
        m_TestEntries[m_uiCurrentTestIndex].m_SubTests[uiSubTestIndex].m_bEnableTest = false;
        nsTestFramework::AutoSaveTestOrder();
      }
    }
    else
    {
      nsTestFramework::Output(nsTestOutput::Error, "Sub-Test '%s' failed: %i Errors (%.2f sec).", szSubTestName, (nsUInt32)m_Result.GetErrorMessageCount(m_uiCurrentTestIndex, uiSubTestIndex), m_fTotalSubTestDuration / 1000.0f);
    }
  }
}

void nsTestFramework::SetSubTestStatusImpl(nsUInt32 uiSubTestIndex, const char* szStatus)
{
  std::scoped_lock _(m_OutputMutex);

  if (m_uiCurrentTestIndex != nsInvalidIndex && uiSubTestIndex != nsInvalidIndex)
  {
    const nsSubTestEntry& subtest = m_TestEntries[m_uiCurrentTestIndex].m_SubTests[uiSubTestIndex];

    m_Result.SetCustomStatus(m_uiCurrentTestIndex, uiSubTestIndex, szStatus);

    if (!nsStringUtils::IsNullOrEmpty(szStatus))
    {
      nsTestFramework::Output(nsTestOutput::Details, "Status of sub-test '%s': %s.", subtest.m_szSubTestName, szStatus);
    }
  }
}

void nsTestFramework::FlushAsserts()
{
  std::scoped_lock _(m_OutputMutex);
  m_Result.AddAsserts(m_uiCurrentTestIndex, m_uiCurrentSubTestIndex, s_iAssertCounter);
  s_iAssertCounter = 0;
}

void nsTestFramework::ScheduleImageComparison(nsUInt32 uiImageNumber, nsUInt32 uiMaxError)
{
  m_bImageComparisonScheduled = true;
  m_uiMaxImageComparisonError = uiMaxError;
  m_uiComparisonImageNumber = uiImageNumber;
}

void nsTestFramework::ScheduleDepthImageComparison(nsUInt32 uiImageNumber, nsUInt32 uiMaxError)
{
  m_bDepthImageComparisonScheduled = true;
  m_uiMaxDepthImageComparisonError = uiMaxError;
  m_uiComparisonDepthImageNumber = uiImageNumber;
}

void nsTestFramework::GenerateComparisonImageName(nsUInt32 uiImageNumber, nsStringBuilder& ref_sImgName)
{
  nsTestEntry* pMainTest = GetTest(GetCurrentTestIndex());

  const char* szTestName = pMainTest->m_szTestName;
  const nsSubTestEntry& subTest = pMainTest->m_SubTests[GetCurrentSubTestIndex()];
  pMainTest->m_pTest->MapImageNumberToString(szTestName, subTest, uiImageNumber, ref_sImgName);
}

void nsTestFramework::GetCurrentComparisonImageName(nsStringBuilder& ref_sImgName)
{
  GenerateComparisonImageName(m_uiComparisonImageNumber, ref_sImgName);
}

void nsTestFramework::SetImageReferenceFolderName(const char* szFolderName)
{
  m_sImageReferenceFolderName = szFolderName;
}

void nsTestFramework::SetImageReferenceOverrideFolderName(const char* szFolderName)
{
  m_sImageReferenceOverrideFolderName = szFolderName;

  if (!m_sImageReferenceOverrideFolderName.empty())
  {
    Output(nsTestOutput::Message, "Using ImageReference override folder '%s'", szFolderName);
  }
}

void nsTestFramework::WriteImageDiffHtml(const char* szFileName, const nsImage& referenceImgRgb, const nsImage& referenceImgAlpha, const nsImage& capturedImgRgb, const nsImage& capturedImgAlpha, const nsImage& diffImgRgb, const nsImage& diffImgAlpha, nsUInt32 uiError, nsUInt32 uiThreshold, nsUInt8 uiMinDiffRgb, nsUInt8 uiMaxDiffRgb,
  nsUInt8 uiMinDiffAlpha, nsUInt8 uiMaxDiffAlpha)
{
  nsFileWriter outputFile;
  if (outputFile.Open(szFileName).Failed())
  {
    nsTestFramework::Output(nsTestOutput::Warning, "Could not open HTML diff file \"%s\" for writing.", szFileName);
    return;
  }

  const char* szTestName = GetTest(GetCurrentTestIndex())->m_szTestName;
  const char* szSubTestName = GetTest(GetCurrentTestIndex())->m_SubTests[GetCurrentSubTestIndex()].m_szSubTestName;

  nsStringBuilder tmp(szTestName, " - ", szSubTestName);

  nsStringBuilder output;
  nsImageUtils::CreateImageDiffHtml(output, tmp, referenceImgRgb, referenceImgAlpha, capturedImgRgb, capturedImgAlpha, diffImgRgb, diffImgAlpha, uiError, uiThreshold, uiMinDiffRgb, uiMaxDiffRgb, uiMinDiffAlpha, uiMaxDiffAlpha);

  if (m_ImageDiffExtraInfoCallback)
  {
    tmp.Clear();

    nsDynamicArray<std::pair<nsString, nsString>> extraInfo = m_ImageDiffExtraInfoCallback();

    for (const auto& labelValuePair : extraInfo)
    {
      tmp.AppendFormat("<tr>\n"
                       "<td>{}:</td>\n"
                       "<td align=\"right\" style=\"padding-left: 2em;\">{}</td>\n"
                       "</tr>\n",
        labelValuePair.first, labelValuePair.second);
    }

    output.ReplaceFirst("<!-- STATS-TABLE-START -->", tmp);
  }

  outputFile.WriteBytes(output.GetData(), output.GetElementCount()).AssertSuccess();
  outputFile.Close();
}

bool nsTestFramework::PerformImageComparison(nsStringBuilder sImgName, const nsImage& img, nsUInt32 uiMaxError, bool bIsLineImage, char* szErrorMsg)
{
  nsImage imgRgba;
  if (nsImageConversion::Convert(img, imgRgba, nsImageFormat::R8G8B8A8_UNORM).Failed())
  {
    safeprintf(szErrorMsg, s_iMaxErrorMessageLength, "Captured Image '%s' could not be converted to RGBA8", sImgName.GetData());
    return false;
  }

  nsStringBuilder sImgPathReference, sImgPathResult;

  if (!m_sImageReferenceOverrideFolderName.empty())
  {
    sImgPathReference = m_sImageReferenceOverrideFolderName.c_str();
    sImgPathReference.AppendPath(sImgName);
    sImgPathReference.ChangeFileExtension(".png");

    if (!nsFileSystem::ExistsFile(sImgPathReference))
    {
      // try the regular path
      sImgPathReference.Clear();
    }
  }

  if (sImgPathReference.IsEmpty())
  {
    sImgPathReference = m_sImageReferenceFolderName.c_str();
    sImgPathReference.AppendPath(sImgName);
    sImgPathReference.ChangeFileExtension(".png");
  }

  sImgPathResult = ":imgout/Images_Result";
  sImgPathResult.AppendPath(sImgName);
  sImgPathResult.ChangeFileExtension(".png");

  auto SaveResultImage = [&]()
  {
    imgRgba.SaveTo(sImgPathResult).IgnoreResult();

#if NS_ENABLED(NS_PLATFORM_WINDOWS_DESKTOP)
    nsStringBuilder sAbsPath;
    if (nsFileSystem::ResolvePath(sImgPathResult, &sAbsPath, nullptr).Failed())
    {
      nsLog::Warning("Failed to resolve absolute path of '{}'. Image will not be compressed with optipng.", sImgPathResult);
      return;
    }

    nsStringBuilder sOptiPng = nsFileSystem::GetSdkRootDirectory();
    sOptiPng.AppendPath("Data/Tools/Precompiled/optipng/optipng.exe");

    if (nsOSFile::ExistsFile(sOptiPng))
    {
      nsProcessOptions opt;
      opt.m_sProcess = sOptiPng;
      opt.m_Arguments.PushBack(sAbsPath);
      nsInt32 iReturnCode = 0;
      if (nsProcess::Execute(opt, &iReturnCode).Failed() || iReturnCode != 0)
      {
        nsLog::Warning("Failed to run optipng with return code {}. Image will not be compressed with optipng.", iReturnCode);
      }
    }
#endif
  };

  // if a previous output image exists, get rid of it
  nsFileSystem::DeleteFile(sImgPathResult);

  nsImage imgExp, imgExpRgba;
  if (imgExp.LoadFrom(sImgPathReference).Failed())
  {
    SaveResultImage();

    safeprintf(szErrorMsg, s_iMaxErrorMessageLength, "Comparison Image '%s' could not be read", sImgPathReference.GetData());
    return false;
  }

  if (nsImageConversion::Convert(imgExp, imgExpRgba, nsImageFormat::R8G8B8A8_UNORM).Failed())
  {
    SaveResultImage();

    safeprintf(szErrorMsg, s_iMaxErrorMessageLength, "Comparison Image '%s' could not be converted to RGBA8", sImgPathReference.GetData());
    return false;
  }

  if (imgRgba.GetWidth() != imgExpRgba.GetWidth() || imgRgba.GetHeight() != imgExpRgba.GetHeight())
  {
    SaveResultImage();

    safeprintf(szErrorMsg, s_iMaxErrorMessageLength, "Comparison Image '%s' size (%ix%i) does not match captured image size (%ix%i)", sImgPathReference.GetData(), imgExpRgba.GetWidth(), imgExpRgba.GetHeight(), imgRgba.GetWidth(), imgRgba.GetHeight());
    return false;
  }

  nsImage imgDiffRgba;
  if (bIsLineImage)
    nsImageUtils::ComputeImageDifferenceABSRelaxed(imgExpRgba, imgRgba, imgDiffRgba);
  else
    nsImageUtils::ComputeImageDifferenceABS(imgExpRgba, imgRgba, imgDiffRgba);

  const nsUInt32 uiMeanError = nsImageUtils::ComputeMeanSquareError(imgDiffRgba, 32);

  if (uiMeanError > uiMaxError)
  {
    SaveResultImage();

    nsUInt8 uiMinDiffRgb, uiMaxDiffRgb, uiMinDiffAlpha, uiMaxDiffAlpha;
    nsImageUtils::Normalize(imgDiffRgba, uiMinDiffRgb, uiMaxDiffRgb, uiMinDiffAlpha, uiMaxDiffAlpha);

    nsImage imgDiffRgb;
    nsImageConversion::Convert(imgDiffRgba, imgDiffRgb, nsImageFormat::R8G8B8_UNORM).IgnoreResult();

    nsStringBuilder sImgDiffName;
    sImgDiffName.SetFormat(":imgout/Images_Diff/{0}.png", sImgName);
    imgDiffRgb.SaveTo(sImgDiffName).IgnoreResult();

    nsImage imgDiffAlpha;
    nsImageUtils::ExtractAlphaChannel(imgDiffRgba, imgDiffAlpha);

    nsStringBuilder sImgDiffAlphaName;
    sImgDiffAlphaName.SetFormat(":imgout/Images_Diff/{0}_alpha.png", sImgName);
    imgDiffAlpha.SaveTo(sImgDiffAlphaName).IgnoreResult();

    nsImage imgExpRgb;
    nsImageConversion::Convert(imgExpRgba, imgExpRgb, nsImageFormat::R8G8B8_UNORM).IgnoreResult();
    nsImage imgExpAlpha;
    nsImageUtils::ExtractAlphaChannel(imgExpRgba, imgExpAlpha);

    nsImage imgRgb;
    nsImageConversion::Convert(imgRgba, imgRgb, nsImageFormat::R8G8B8_UNORM).IgnoreResult();
    nsImage imgAlpha;
    nsImageUtils::ExtractAlphaChannel(imgRgba, imgAlpha);

    nsStringBuilder sDiffHtmlPath;
    sDiffHtmlPath.SetFormat(":imgout/Html_Diff/{0}.html", sImgName);
    WriteImageDiffHtml(sDiffHtmlPath, imgExpRgb, imgExpAlpha, imgRgb, imgAlpha, imgDiffRgb, imgDiffAlpha, uiMeanError, uiMaxError, uiMinDiffRgb, uiMaxDiffRgb, uiMinDiffAlpha, uiMaxDiffAlpha);

    safeprintf(szErrorMsg, s_iMaxErrorMessageLength, "Error: Image Comparison Failed: MSE of %u exceeds threshold of %u for image '%s'.", uiMeanError, uiMaxError, sImgName.GetData());

    nsStringBuilder sDataDirRelativePath;
    nsFileSystem::ResolvePath(sDiffHtmlPath, nullptr, &sDataDirRelativePath).IgnoreResult();
    nsTestFramework::Output(nsTestOutput::ImageDiffFile, sDataDirRelativePath);
    return false;
  }
  return true;
}

bool nsTestFramework::CompareImages(nsUInt32 uiImageNumber, nsUInt32 uiMaxError, char* szErrorMsg, bool bIsDepthImage, bool bIsLineImage)
{
  nsStringBuilder sImgName;
  GenerateComparisonImageName(uiImageNumber, sImgName);

  nsImage img;
  if (bIsDepthImage)
  {
    sImgName.Append("-depth");
    if (GetTest(GetCurrentTestIndex())->m_pTest->GetDepthImage(img, *GetCurrentSubTest(), uiImageNumber).Failed())
    {
      safeprintf(szErrorMsg, s_iMaxErrorMessageLength, "Depth image '%s' could not be captured", sImgName.GetData());
      return false;
    }
  }
  else
  {
    if (GetTest(GetCurrentTestIndex())->m_pTest->GetImage(img, *GetCurrentSubTest(), uiImageNumber).Failed())
    {
      safeprintf(szErrorMsg, s_iMaxErrorMessageLength, "Image '%s' could not be captured", sImgName.GetData());
      return false;
    }
  }

  bool bImagesMatch = true;
  if (img.GetNumArrayIndices() <= 1)
  {
    bImagesMatch = PerformImageComparison(sImgName, img, uiMaxError, bIsLineImage, szErrorMsg);
  }
  else
  {
    nsStringBuilder lastError;
    for (nsUInt32 i = 0; i < img.GetNumArrayIndices(); ++i)
    {
      nsStringBuilder subImageName;
      subImageName.AppendFormat("{0}_{1}", sImgName, i);
      if (!PerformImageComparison(subImageName, img.GetSubImageView(0, 0, i), uiMaxError, bIsLineImage, szErrorMsg))
      {
        bImagesMatch = false;
        if (!lastError.IsEmpty())
        {
          nsTestFramework::Output(nsTestOutput::Error, "%s", lastError.GetData());
        }
        lastError = szErrorMsg;
      }
    }
  }

  if (m_ImageComparisonCallback)
  {
    m_ImageComparisonCallback(bImagesMatch);
  }

  return bImagesMatch;
}

void nsTestFramework::SetImageComparisonCallback(const ImageComparisonCallback& callback)
{
  m_ImageComparisonCallback = callback;
}

nsResult nsTestFramework::CaptureRegressionStat(nsStringView sTestName, nsStringView sName, nsStringView sUnit, float value, nsInt32 iTestId)
{
  nsStringBuilder strippedTestName = sTestName;
  strippedTestName.ReplaceAll(" ", "");

  nsStringBuilder perTestName;
  if (iTestId < 0)
  {
    perTestName.SetFormat("{}_{}", strippedTestName, sName);
  }
  else
  {
    perTestName.SetFormat("{}_{}_{}", strippedTestName, sName, iTestId);
  }

  {
    nsStringBuilder regression;
    // The 6 floating point digits are forced as per a requirement of the CI
    // feature that parses these values.
    regression.SetFormat("[test][REGRESSION:{}:{}:{}]", perTestName, sUnit, nsArgF(value, 6));
    nsLog::Info(regression);
  }

  return NS_SUCCESS;
}

////////////////////////////////////////////////////////////////////////
// nsTestFramework static functions
////////////////////////////////////////////////////////////////////////

void nsTestFramework::Output(nsTestOutput::Enum type, const char* szMsg, ...)
{
  va_list args;
  va_start(args, szMsg);

  OutputArgs(type, szMsg, args);

  va_end(args);
}

void nsTestFramework::OutputArgs(nsTestOutput::Enum type, const char* szMsg, va_list szArgs)
{
  // format the output text
  char szBuffer[1024 * 10];
  nsInt32 pos = 0;

  if (nsTestFramework::s_LogTimestampMode != nsLog::TimestampMode::None)
  {
    if (type == nsTestOutput::BeginBlock || type == nsTestOutput::EndBlock || type == nsTestOutput::ImportantInfo || type == nsTestOutput::Details || type == nsTestOutput::Success || type == nsTestOutput::Message || type == nsTestOutput::Warning || type == nsTestOutput::Error ||
        type == nsTestOutput::FinalResult)
    {
      nsStringBuilder timestamp;

      nsLog::GenerateFormattedTimestamp(nsTestFramework::s_LogTimestampMode, timestamp);
      pos = nsStringUtils::snprintf(szBuffer, NS_ARRAY_SIZE(szBuffer), "%s", timestamp.GetData());
    }
  }
  nsStringUtils::vsnprintf(szBuffer + pos, NS_ARRAY_SIZE(szBuffer) - pos, szMsg, szArgs);

  GetInstance()->OutputImpl(type, szBuffer);
}

void nsTestFramework::Error(const char* szError, const char* szFile, nsInt32 iLine, const char* szFunction, nsStringView sMsg, ...)
{
  va_list args;
  va_start(args, sMsg);

  Error(szError, szFile, iLine, szFunction, sMsg, args);

  va_end(args);
}

void nsTestFramework::Error(const char* szError, const char* szFile, nsInt32 iLine, const char* szFunction, nsStringView sMsg, va_list szArgs)
{
  // format the output text
  char szBuffer[1024 * 10];
  nsStringUtils::vsnprintf(szBuffer, NS_ARRAY_SIZE(szBuffer), nsString(sMsg).GetData(), szArgs);

  GetInstance()->ErrorImpl(szError, szFile, iLine, szFunction, szBuffer);
}

void nsTestFramework::TestResult(nsUInt32 uiSubTestIndex, bool bSuccess, double fDuration)
{
  GetInstance()->TestResultImpl(uiSubTestIndex, bSuccess, fDuration);
}

void nsTestFramework::SetSubTestStatus(nsUInt32 uiSubTestIndex, const char* szStatus)
{
  GetInstance()->SetSubTestStatusImpl(uiSubTestIndex, szStatus);
}

////////////////////////////////////////////////////////////////////////
// NS_TEST_... macro functions
////////////////////////////////////////////////////////////////////////

#define OUTPUT_TEST_ERROR                                                        \
  {                                                                              \
    va_list args;                                                                \
    va_start(args, szMsg);                                                       \
    nsTestFramework::Error(szErrorText, szFile, iLine, szFunction, szMsg, args); \
    NS_TEST_DEBUG_BREAK                                                          \
    va_end(args);                                                                \
    return false;                                                                \
  }

#define OUTPUT_TEST_ERROR_NO_BREAK                                               \
  {                                                                              \
    va_list args;                                                                \
    va_start(args, szMsg);                                                       \
    nsTestFramework::Error(szErrorText, szFile, iLine, szFunction, szMsg, args); \
    va_end(args);                                                                \
    return false;                                                                \
  }

bool nsTestBool(bool bCondition, const char* szErrorText, const char* szFile, nsInt32 iLine, const char* szFunction, const char* szMsg, ...)
{
  nsTestFramework::s_iAssertCounter++;

  if (!bCondition)
  {
    // if the test breaks here, go one up in the callstack to see where it exactly failed
    OUTPUT_TEST_ERROR
  }

  return true;
}

bool nsTestResult(nsResult condition, const char* szErrorText, const char* szFile, nsInt32 iLine, const char* szFunction, const char* szMsg, ...)
{
  nsTestFramework::s_iAssertCounter++;

  if (condition.Failed())
  {
    // if the test breaks here, go one up in the callstack to see where it exactly failed
    OUTPUT_TEST_ERROR
  }

  return true;
}

bool nsTestDouble(double f1, double f2, double fEps, const char* szF1, const char* szF2, const char* szFile, nsInt32 iLine, const char* szFunction, const char* szMsg, ...)
{
  nsTestFramework::s_iAssertCounter++;

  const double fD = f1 - f2;

  if (fD < -fEps || fD > +fEps)
  {
    char szErrorText[256];
    safeprintf(szErrorText, 256, "Failure: '%s' (%.8f) does not equal '%s' (%.8f) within an epsilon of %.8f", szF1, f1, szF2, f2, fEps);

    OUTPUT_TEST_ERROR
  }

  return true;
}

bool nsTestInt(nsInt64 i1, nsInt64 i2, const char* szI1, const char* szI2, const char* szFile, nsInt32 iLine, const char* szFunction, const char* szMsg, ...)
{
  nsTestFramework::s_iAssertCounter++;

  if (i1 != i2)
  {
    char szErrorText[256];
    safeprintf(szErrorText, 256, "Failure: '%s' (%lli) does not equal '%s' (%lli)", szI1, i1, szI2, i2);

    OUTPUT_TEST_ERROR
  }

  return true;
}

bool nsTestWString(std::wstring s1, std::wstring s2, const char* szWString1, const char* szWString2, const char* szFile, nsInt32 iLine, const char* szFunction, const char* szMsg, ...)
{
  nsTestFramework::s_iAssertCounter++;

  if (s1 != s2)
  {
    char szErrorText[2048];
    safeprintf(szErrorText, 2048, "Failure: '%s' (%s) does not equal '%s' (%s)", szWString1, nsStringUtf8(s1.c_str()).GetData(), szWString2, nsStringUtf8(s2.c_str()).GetData());

    OUTPUT_TEST_ERROR
  }

  return true;
}

bool nsTestString(nsStringView s1, nsStringView s2, const char* szString1, const char* szString2, const char* szFile, nsInt32 iLine, const char* szFunction, const char* szMsg, ...)
{
  nsTestFramework::s_iAssertCounter++;

  if (s1 != s2)
  {
    nsStringBuilder ss1 = s1;
    nsStringBuilder ss2 = s2;

    char szErrorText[2048];
    safeprintf(szErrorText, 2048, "Failure: '%s' (%s) does not equal '%s' (%s)", szString1, ss1.GetData(), szString2, ss2.GetData());

    OUTPUT_TEST_ERROR
  }

  return true;
}

bool nsTestVector(nsVec4d v1, nsVec4d v2, double fEps, const char* szCondition, const char* szFile, nsInt32 iLine, const char* szFunction, const char* szMsg, ...)
{
  nsTestFramework::s_iAssertCounter++;

  char szErrorText[256];

  if (!nsMath::IsEqual(v1.x, v2.x, fEps))
  {
    safeprintf(szErrorText, 256, "Failure: '%s' - v1.x (%.8f) does not equal v2.x (%.8f) within an epsilon of %.8f", szCondition, v1.x, v2.x, fEps);

    OUTPUT_TEST_ERROR
  }

  if (!nsMath::IsEqual(v1.y, v2.y, fEps))
  {
    safeprintf(szErrorText, 256, "Failure: '%s' - v1.y (%.8f) does not equal v2.y (%.8f) within an epsilon of %.8f", szCondition, v1.y, v2.y, fEps);

    OUTPUT_TEST_ERROR
  }

  if (!nsMath::IsEqual(v1.z, v2.z, fEps))
  {
    safeprintf(szErrorText, 256, "Failure: '%s' - v1.z (%.8f) does not equal v2.z (%.8f) within an epsilon of %.8f", szCondition, v1.z, v2.z, fEps);

    OUTPUT_TEST_ERROR
  }

  if (!nsMath::IsEqual(v1.w, v2.w, fEps))
  {
    safeprintf(szErrorText, 256, "Failure: '%s' - v1.w (%.8f) does not equal v2.w (%.8f) within an epsilon of %.8f", szCondition, v1.w, v2.w, fEps);

    OUTPUT_TEST_ERROR
  }

  return true;
}

bool nsTestFiles(const char* szFile1, const char* szFile2, const char* szFile, nsInt32 iLine, const char* szFunction, const char* szMsg, ...)
{
  nsTestFramework::s_iAssertCounter++;

  char szErrorText[s_iMaxErrorMessageLength];

  nsFileReader ReadFile1;
  nsFileReader ReadFile2;

  if (ReadFile1.Open(szFile1) == NS_FAILURE)
  {
    safeprintf(szErrorText, s_iMaxErrorMessageLength, "Failure: File '%s' could not be read.", szFile1);

    OUTPUT_TEST_ERROR
  }
  else if (ReadFile2.Open(szFile2) == NS_FAILURE)
  {
    safeprintf(szErrorText, s_iMaxErrorMessageLength, "Failure: File '%s' could not be read.", szFile2);

    OUTPUT_TEST_ERROR
  }

  else if (ReadFile1.GetFileSize() != ReadFile2.GetFileSize())
  {
    safeprintf(szErrorText, s_iMaxErrorMessageLength, "Failure: File sizes do not match: '%s' (%llu Bytes) and '%s' (%llu Bytes)", szFile1, ReadFile1.GetFileSize(), szFile2, ReadFile2.GetFileSize());

    OUTPUT_TEST_ERROR
  }
  else
  {
    while (true)
    {
      nsUInt8 uiTemp1[512];
      nsUInt8 uiTemp2[512];
      const nsUInt64 uiRead1 = ReadFile1.ReadBytes(uiTemp1, 512);
      const nsUInt64 uiRead2 = ReadFile2.ReadBytes(uiTemp2, 512);

      if (uiRead1 != uiRead2)
      {
        safeprintf(szErrorText, s_iMaxErrorMessageLength, "Failure: Files could not read same amount of data: '%s' and '%s'", szFile1, szFile2);

        OUTPUT_TEST_ERROR
      }
      else
      {
        if (uiRead1 == 0)
          break;

        if (memcmp(uiTemp1, uiTemp2, (size_t)uiRead1) != 0)
        {
          safeprintf(szErrorText, s_iMaxErrorMessageLength, "Failure: Files contents do not match: '%s' and '%s'", szFile1, szFile2);

          OUTPUT_TEST_ERROR
        }
      }
    }
  }

  return true;
}

bool nsTestTextFiles(const char* szFile1, const char* szFile2, const char* szFile, nsInt32 iLine, const char* szFunction, const char* szMsg, ...)
{
  nsTestFramework::s_iAssertCounter++;

  char szErrorText[s_iMaxErrorMessageLength];

  nsFileReader ReadFile1;
  nsFileReader ReadFile2;

  if (ReadFile1.Open(szFile1) == NS_FAILURE)
  {
    safeprintf(szErrorText, s_iMaxErrorMessageLength, "Failure: File '%s' could not be read.", szFile1);

    OUTPUT_TEST_ERROR
  }
  else if (ReadFile2.Open(szFile2) == NS_FAILURE)
  {
    safeprintf(szErrorText, s_iMaxErrorMessageLength, "Failure: File '%s' could not be read.", szFile2);

    OUTPUT_TEST_ERROR
  }
  else
  {
    nsStringBuilder sFile1;
    sFile1.ReadAll(ReadFile1);
    sFile1.ReplaceAll("\r\n", "\n");

    nsStringBuilder sFile2;
    sFile2.ReadAll(ReadFile2);
    sFile2.ReplaceAll("\r\n", "\n");

    if (sFile1 != sFile2)
    {
      safeprintf(szErrorText, s_iMaxErrorMessageLength, "Failure: Text files contents do not match: '%s' and '%s'", szFile1, szFile2);

      OUTPUT_TEST_ERROR
    }
  }

  return true;
}

bool nsTestImage(nsUInt32 uiImageNumber, nsUInt32 uiMaxError, bool bIsDepthImage, bool bIsLineImage, const char* szFile, nsInt32 iLine, const char* szFunction, const char* szMsg, ...)
{
  char szErrorText[s_iMaxErrorMessageLength] = "";

  if (!nsTestFramework::GetInstance()->CompareImages(uiImageNumber, uiMaxError, szErrorText, bIsDepthImage, bIsLineImage))
  {
    OUTPUT_TEST_ERROR_NO_BREAK
  }

  return true;
}
