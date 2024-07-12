#pragma once

#include <TestFramework/Framework/Declarations.h>
#include <TestFramework/TestFrameworkDLL.h>
#include <deque>
#include <string>
#include <vector>

struct NS_TEST_DLL nsTestOutput
{
  /// \brief Defines the type of output message for nsTestOutputMessage.
  enum Enum
  {
    InvalidType = -1,
    StartOutput = 0,
    BeginBlock,
    EndBlock,
    ImportantInfo,
    Details,
    Success,
    Message,
    Warning,
    Error,
    ImageDiffFile,
    Duration,
    FinalResult,
    AllOutputTypes
  };

  static const char* const s_Names[];
  static const char* ToString(Enum type);
  static Enum FromString(const char* szName);
};

/// \brief A message of type nsTestOutput::Enum, stored in nsResult.
struct NS_TEST_DLL nsTestErrorMessage
{
  nsTestErrorMessage() = default;

  std::string m_sError;
  std::string m_sBlock;
  std::string m_sFile;
  nsInt32 m_iLine = -1;
  std::string m_sFunction;
  std::string m_sMessage;
};

/// \brief A message of type nsTestOutput::Enum, stored in nsResult.
struct NS_TEST_DLL nsTestOutputMessage
{
  nsTestOutputMessage() = default;

  nsTestOutput::Enum m_Type = nsTestOutput::ImportantInfo;
  std::string m_sMessage;
  nsInt32 m_iErrorIndex = -1;
};

struct NS_TEST_DLL nsTestResultQuery
{
  /// \brief Defines what information should be accumulated over the sub-tests in nsTestEntry::GetSubTestCount.
  enum Enum
  {
    Count,
    Executed,
    Success,
    Errors,
  };
};

/// \brief Stores the results of a test run. Used by both nsTestEntry and nsSubTestEntry.
struct NS_TEST_DLL nsTestResultData
{
  nsTestResultData() = default;

  void Reset();
  void AddOutput(nsInt32 iOutputIndex);

  std::string m_sName;
  bool m_bExecuted = false;     ///< Whether the test was executed. If false, the test was either deactivated or the test process crashed before
                                ///< executing it.
  bool m_bSuccess = false;      ///< Whether the test succeeded or not.
  int m_iTestAsserts = 0;       ///< Asserts that were checked. For tests this includes the count of all of their sub-tests as well.
  double m_fTestDuration = 0.0; ///< Duration of the test/sub-test. For tests, this includes the duration of all their sub-tests as well.
  nsInt32 m_iFirstOutput = -1;  ///< First output message. For tests, this range includes all messages of their sub-tests as well.
  nsInt32 m_iLastOutput = -1;   ///< Last output message. For tests, this range includes all messages of their sub-tests as well.
  std::string m_sCustomStatus;  ///< If this is not empty, the UI will display this instead of "Pending"
};

struct NS_TEST_DLL nsTestConfiguration
{
  nsTestConfiguration();

  nsUInt64 m_uiInstalledMainMemory = 0;
  nsUInt32 m_uiMemoryPageSize = 0;
  nsUInt32 m_uiCPUCoreCount = 0;
  bool m_b64BitOS = false;
  bool m_b64BitApplication = false;
  std::string m_sPlatformName;
  std::string m_sBuildConfiguration; ///< Debug, Release, etc
  nsInt64 m_iDateTime = 0;           ///< in seconds since Linux epoch
  nsInt32 m_iRCSRevision = -1;
  std::string m_sHostName;
};

class NS_TEST_DLL nsTestFrameworkResult
{
public:
  nsTestFrameworkResult() = default;

  // Manage tests
  void Clear();
  void SetupTests(const std::deque<nsTestEntry>& tests, const nsTestConfiguration& config);
  void Reset();
  bool WriteJsonToFile(const char* szFileName) const;

  // Result access
  nsUInt32 GetTestCount(nsTestResultQuery::Enum countQuery = nsTestResultQuery::Count) const;
  nsUInt32 GetSubTestCount(nsUInt32 uiTestIndex, nsTestResultQuery::Enum countQuery = nsTestResultQuery::Count) const;
  nsUInt32 GetTestIndexByName(const char* szTestName) const;
  nsUInt32 GetSubTestIndexByName(nsUInt32 uiTestIndex, const char* szSubTestName) const;
  double GetTotalTestDuration() const;
  const nsTestResultData& GetTestResultData(nsUInt32 uiTestIndex, nsUInt32 uiSubTestIndex) const;

  // Test output
  void TestOutput(nsUInt32 uiTestIndex, nsUInt32 uiSubTestIndex, nsTestOutput::Enum type, const char* szMsg);
  void TestError(nsUInt32 uiTestIndex, nsUInt32 uiSubTestIndex, const char* szError, const char* szBlock, const char* szFile, nsInt32 iLine, const char* szFunction, const char* szMsg);
  void TestResult(nsUInt32 uiTestIndex, nsUInt32 uiSubTestIndex, bool bSuccess, double fDuration);
  void AddAsserts(nsUInt32 uiTestIndex, nsUInt32 uiSubTestIndex, int iCount);
  void SetCustomStatus(nsUInt32 uiTestIndex, nsUInt32 uiSubTestIndex, const char* szCustomStatus);

  // Messages / Errors
  nsUInt32 GetOutputMessageCount(nsUInt32 uiTestIndex = nsInvalidIndex, nsUInt32 uiSubTestIndex = nsInvalidIndex, nsTestOutput::Enum type = nsTestOutput::AllOutputTypes) const;
  const nsTestOutputMessage* GetOutputMessage(nsUInt32 uiOutputMessageIdx) const;

  nsUInt32 GetErrorMessageCount(nsUInt32 uiTestIndex = nsInvalidIndex, nsUInt32 uiSubTestIndex = nsInvalidIndex) const;
  const nsTestErrorMessage* GetErrorMessage(nsUInt32 uiErrorMessageIdx) const;

private:
  struct nsSubTestResult
  {
    nsSubTestResult() = default;
    nsSubTestResult(const char* szName) { m_Result.m_sName = szName; }

    nsTestResultData m_Result;
  };

  struct nsTestResult
  {
    nsTestResult() = default;
    nsTestResult(const char* szName) { m_Result.m_sName = szName; }

    void Reset();

    nsTestResultData m_Result;
    std::deque<nsSubTestResult> m_SubTests;
  };

private:
  nsTestConfiguration m_Config;
  std::deque<nsTestResult> m_Tests;
  std::deque<nsTestErrorMessage> m_Errors;
  std::deque<nsTestOutputMessage> m_TestOutput;
};
