#include <TestFramework/TestFrameworkPCH.h>

#include <Foundation/Types/ScopeExit.h>
#include <TestFramework/Framework/TestResults.h>

////////////////////////////////////////////////////////////////////////
// nsTestOutput public functions
////////////////////////////////////////////////////////////////////////

const char* const nsTestOutput::s_Names[] = {
  "StartOutput", "BeginBlock", "EndBlock", "ImportantInfo", "Details", "Success", "Message", "Warning", "Error", "Duration", "FinalResult"};

const char* nsTestOutput::ToString(Enum type)
{
  return s_Names[type];
}

nsTestOutput::Enum nsTestOutput::FromString(const char* szName)
{
  for (nsUInt32 i = 0; i < AllOutputTypes; ++i)
  {
    if (strcmp(szName, s_Names[i]) == 0)
      return (nsTestOutput::Enum)i;
  }
  return InvalidType;
}


////////////////////////////////////////////////////////////////////////
// nsTestResultData public functions
////////////////////////////////////////////////////////////////////////

void nsTestResultData::Reset()
{
  m_bExecuted = false;
  m_bSuccess = false;
  m_iTestAsserts = 0;
  m_fTestDuration = 0.0;
  m_iFirstOutput = -1;
  m_iLastOutput = -1;
  m_sCustomStatus.clear();
}

void nsTestResultData::AddOutput(nsInt32 iOutputIndex)
{
  if (m_iFirstOutput == -1)
  {
    m_iFirstOutput = iOutputIndex;
    m_iLastOutput = iOutputIndex;
  }
  else
  {
    m_iLastOutput = iOutputIndex;
  }
}


////////////////////////////////////////////////////////////////////////
// nsTestResultData public functions
////////////////////////////////////////////////////////////////////////

nsTestConfiguration::nsTestConfiguration()

  = default;


////////////////////////////////////////////////////////////////////////
// nsTestFrameworkResult public functions
////////////////////////////////////////////////////////////////////////

void nsTestFrameworkResult::Clear()
{
  m_Tests.clear();
  m_Errors.clear();
  m_TestOutput.clear();
}

void nsTestFrameworkResult::SetupTests(const std::deque<nsTestEntry>& tests, const nsTestConfiguration& config)
{
  m_Config = config;
  Clear();

  const nsUInt32 uiTestCount = (nsUInt32)tests.size();
  for (nsUInt32 uiTestIdx = 0; uiTestIdx < uiTestCount; ++uiTestIdx)
  {
    m_Tests.push_back(nsTestResult(tests[uiTestIdx].m_szTestName));

    const nsUInt32 uiSubTestCount = (nsUInt32)tests[uiTestIdx].m_SubTests.size();
    for (nsUInt32 uiSubTestIdx = 0; uiSubTestIdx < uiSubTestCount; ++uiSubTestIdx)
    {
      m_Tests[uiTestIdx].m_SubTests.push_back(nsSubTestResult(tests[uiTestIdx].m_SubTests[uiSubTestIdx].m_szSubTestName));
    }
  }
}

void ::nsTestFrameworkResult::Reset()
{
  const nsUInt32 uiTestCount = (nsUInt32)m_Tests.size();
  for (nsUInt32 uiTestIdx = 0; uiTestIdx < uiTestCount; ++uiTestIdx)
  {
    m_Tests[uiTestIdx].Reset();
  }
  m_Errors.clear();
  m_TestOutput.clear();
}

bool nsTestFrameworkResult::WriteJsonToFile(const char* szFileName) const
{
  nsStartup::StartupCoreSystems();
  NS_SCOPE_EXIT(nsStartup::ShutdownCoreSystems());

  {
    nsStringBuilder jsonFilename;
    if (nsPathUtils::IsAbsolutePath(szFileName))
    {
      // Make sure we can access raw absolute file paths
      if (nsFileSystem::AddDataDirectory("", "jsonoutput", ":", nsFileSystem::AllowWrites).Failed())
        return false;

      jsonFilename = szFileName;
    }
    else
    {
      // If this is a relative path, we use the nstest/ data directory to make sure that this works properly with the fileserver.
      if (nsFileSystem::AddDataDirectory(">nstest/", "jsonoutput", ":", nsFileSystem::AllowWrites).Failed())
        return false;

      jsonFilename = ":";
      jsonFilename.AppendPath(szFileName);
    }

    nsFileWriter file;
    if (file.Open(jsonFilename).Failed())
    {
      return false;
    }
    nsStandardJSONWriter js;
    js.SetOutputStream(&file);

    js.BeginObject();
    {
      js.BeginObject("configuration");
      {
        js.AddVariableUInt64("m_uiInstalledMainMemory", m_Config.m_uiInstalledMainMemory);
        js.AddVariableUInt32("m_uiMemoryPageSize", m_Config.m_uiMemoryPageSize);
        js.AddVariableUInt32("m_uiCPUCoreCount", m_Config.m_uiCPUCoreCount);
        js.AddVariableBool("m_b64BitOS", m_Config.m_b64BitOS);
        js.AddVariableBool("m_b64BitApplication", m_Config.m_b64BitApplication);
        js.AddVariableString("m_sPlatformName", m_Config.m_sPlatformName.c_str());
        js.AddVariableString("m_sBuildConfiguration", m_Config.m_sBuildConfiguration.c_str());
        js.AddVariableInt64("m_iDateTime", m_Config.m_iDateTime);
        js.AddVariableInt32("m_iRCSRevision", m_Config.m_iRCSRevision);
        js.AddVariableString("m_sHostName", m_Config.m_sHostName.c_str());
      }
      js.EndObject();

      // Output Messages
      js.BeginArray("messages");
      {
        nsUInt32 uiMessages = GetOutputMessageCount();
        for (nsUInt32 uiMessageIdx = 0; uiMessageIdx < uiMessages; ++uiMessageIdx)
        {
          const nsTestOutputMessage* pMessage = GetOutputMessage(uiMessageIdx);
          js.BeginObject();
          {
            js.AddVariableString("m_Type", nsTestOutput::ToString(pMessage->m_Type));
            js.AddVariableString("m_sMessage", pMessage->m_sMessage.c_str());
            if (pMessage->m_iErrorIndex != -1)
              js.AddVariableInt32("m_iErrorIndex", pMessage->m_iErrorIndex);
          }
          js.EndObject();
        }
      }
      js.EndArray();

      // Error Messages
      js.BeginArray("errors");
      {
        nsUInt32 uiMessages = GetErrorMessageCount();
        for (nsUInt32 uiMessageIdx = 0; uiMessageIdx < uiMessages; ++uiMessageIdx)
        {
          const nsTestErrorMessage* pMessage = GetErrorMessage(uiMessageIdx);
          js.BeginObject();
          {
            js.AddVariableString("m_sError", pMessage->m_sError.c_str());
            js.AddVariableString("m_sBlock", pMessage->m_sBlock.c_str());
            js.AddVariableString("m_sFile", pMessage->m_sFile.c_str());
            js.AddVariableString("m_sFunction", pMessage->m_sFunction.c_str());
            js.AddVariableInt32("m_iLine", pMessage->m_iLine);
            js.AddVariableString("m_sMessage", pMessage->m_sMessage.c_str());
          }
          js.EndObject();
        }
      }
      js.EndArray();

      // Tests
      js.BeginArray("tests");
      {
        nsUInt32 uiTests = GetTestCount();
        for (nsUInt32 uiTestIdx = 0; uiTestIdx < uiTests; ++uiTestIdx)
        {
          const nsTestResultData& testResult = GetTestResultData(uiTestIdx, nsInvalidIndex);
          js.BeginObject();
          {
            js.AddVariableString("m_sName", testResult.m_sName.c_str());
            js.AddVariableBool("m_bExecuted", testResult.m_bExecuted);
            js.AddVariableBool("m_bSuccess", testResult.m_bSuccess);
            js.AddVariableInt32("m_iTestAsserts", testResult.m_iTestAsserts);
            js.AddVariableDouble("m_fTestDuration", testResult.m_fTestDuration);
            js.AddVariableInt32("m_iFirstOutput", testResult.m_iFirstOutput);
            js.AddVariableInt32("m_iLastOutput", testResult.m_iLastOutput);

            // Sub Tests
            js.BeginArray("subTests");
            {
              nsUInt32 uiSubTests = GetSubTestCount(uiTestIdx);
              for (nsUInt32 uiSubTestIdx = 0; uiSubTestIdx < uiSubTests; ++uiSubTestIdx)
              {
                const nsTestResultData& subTestResult = GetTestResultData(uiTestIdx, uiSubTestIdx);
                js.BeginObject();
                {
                  js.AddVariableString("m_sName", subTestResult.m_sName.c_str());
                  js.AddVariableBool("m_bExecuted", subTestResult.m_bExecuted);
                  js.AddVariableBool("m_bSuccess", subTestResult.m_bSuccess);
                  js.AddVariableInt32("m_iTestAsserts", subTestResult.m_iTestAsserts);
                  js.AddVariableDouble("m_fTestDuration", subTestResult.m_fTestDuration);
                  js.AddVariableInt32("m_iFirstOutput", subTestResult.m_iFirstOutput);
                  js.AddVariableInt32("m_iLastOutput", subTestResult.m_iLastOutput);
                }
                js.EndObject();
              }
            }
            js.EndArray(); // subTests
          }
          js.EndObject();
        }
      }
      js.EndArray(); // tests
    }
    js.EndObject();
  }

  return true;
}

nsUInt32 nsTestFrameworkResult::GetTestCount(nsTestResultQuery::Enum countQuery) const
{
  nsUInt32 uiAccumulator = 0;
  const nsUInt32 uiTests = (nsUInt32)m_Tests.size();

  if (countQuery == nsTestResultQuery::Count)
    return uiTests;

  if (countQuery == nsTestResultQuery::Errors)
    return (nsUInt32)m_Errors.size();

  for (nsUInt32 uiTest = 0; uiTest < uiTests; ++uiTest)
  {
    switch (countQuery)
    {
      case nsTestResultQuery::Executed:
        uiAccumulator += m_Tests[uiTest].m_Result.m_bExecuted ? 1 : 0;
        break;
      case nsTestResultQuery::Success:
        uiAccumulator += m_Tests[uiTest].m_Result.m_bSuccess ? 1 : 0;
        break;
      default:
        break;
    }
  }
  return uiAccumulator;
}

nsUInt32 nsTestFrameworkResult::GetSubTestCount(nsUInt32 uiTestIndex, nsTestResultQuery::Enum countQuery) const
{
  if (uiTestIndex >= (nsUInt32)m_Tests.size())
    return 0;

  const nsTestResult& test = m_Tests[uiTestIndex];
  nsUInt32 uiAccumulator = 0;
  const nsUInt32 uiSubTests = (nsUInt32)test.m_SubTests.size();

  if (countQuery == nsTestResultQuery::Count)
    return uiSubTests;

  if (countQuery == nsTestResultQuery::Errors)
  {
    for (nsInt32 iOutputIdx = test.m_Result.m_iFirstOutput; iOutputIdx <= test.m_Result.m_iLastOutput && iOutputIdx != -1; ++iOutputIdx)
    {
      if (m_TestOutput[iOutputIdx].m_Type == nsTestOutput::Error)
        uiAccumulator++;
    }
    return uiAccumulator;
  }

  for (nsUInt32 uiSubTest = 0; uiSubTest < uiSubTests; ++uiSubTest)
  {
    switch (countQuery)
    {
      case nsTestResultQuery::Executed:
        uiAccumulator += test.m_SubTests[uiSubTest].m_Result.m_bExecuted ? 1 : 0;
        break;
      case nsTestResultQuery::Success:
        uiAccumulator += test.m_SubTests[uiSubTest].m_Result.m_bSuccess ? 1 : 0;
        break;
      default:
        break;
    }
  }
  return uiAccumulator;
}

nsUInt32 nsTestFrameworkResult::GetTestIndexByName(const char* szTestName) const
{
  const nsUInt32 uiTestCount = GetTestCount();
  for (nsUInt32 i = 0; i < uiTestCount; ++i)
  {
    if (m_Tests[i].m_Result.m_sName.compare(szTestName) == 0)
      return i;
  }

  return nsInvalidIndex;
}

nsUInt32 nsTestFrameworkResult::GetSubTestIndexByName(nsUInt32 uiTestIndex, const char* szSubTestName) const
{
  if (uiTestIndex >= GetTestCount())
    return nsInvalidIndex;

  const nsUInt32 uiSubTestCount = GetSubTestCount(uiTestIndex);
  for (nsUInt32 i = 0; i < uiSubTestCount; ++i)
  {
    if (m_Tests[uiTestIndex].m_SubTests[i].m_Result.m_sName.compare(szSubTestName) == 0)
      return i;
  }

  return nsInvalidIndex;
}

double nsTestFrameworkResult::GetTotalTestDuration() const
{
  double fTotalTestDuration = 0.0;
  const nsUInt32 uiTests = (nsUInt32)m_Tests.size();
  for (nsUInt32 uiTest = 0; uiTest < uiTests; ++uiTest)
  {
    fTotalTestDuration += m_Tests[uiTest].m_Result.m_fTestDuration;
  }
  return fTotalTestDuration;
}

const nsTestResultData& nsTestFrameworkResult::GetTestResultData(nsUInt32 uiTestIndex, nsUInt32 uiSubTestIndex) const
{
  return (uiSubTestIndex == nsInvalidIndex) ? m_Tests[uiTestIndex].m_Result : m_Tests[uiTestIndex].m_SubTests[uiSubTestIndex].m_Result;
}

void nsTestFrameworkResult::TestOutput(nsUInt32 uiTestIndex, nsUInt32 uiSubTestIndex, nsTestOutput::Enum type, const char* szMsg)
{
  if (uiTestIndex != nsInvalidIndex)
  {
    m_Tests[uiTestIndex].m_Result.AddOutput((nsUInt32)m_TestOutput.size());
    if (uiSubTestIndex != nsInvalidIndex)
    {
      m_Tests[uiTestIndex].m_SubTests[uiSubTestIndex].m_Result.AddOutput((nsUInt32)m_TestOutput.size());
    }
  }

  m_TestOutput.push_back(nsTestOutputMessage());
  nsTestOutputMessage& outputMessage = *m_TestOutput.rbegin();
  outputMessage.m_Type = type;
  outputMessage.m_sMessage.assign(szMsg);
}

void nsTestFrameworkResult::TestError(nsUInt32 uiTestIndex, nsUInt32 uiSubTestIndex, const char* szError, const char* szBlock, const char* szFile,
  nsInt32 iLine, const char* szFunction, const char* szMsg)
{
  // In case there is no message set, we use the error as the message.
  TestOutput(uiTestIndex, uiSubTestIndex, nsTestOutput::Error, szError);
  m_TestOutput.rbegin()->m_iErrorIndex = (nsInt32)m_Errors.size();

  m_Errors.push_back(nsTestErrorMessage());
  nsTestErrorMessage& errorMessage = *m_Errors.rbegin();
  errorMessage.m_sError.assign(szError);
  errorMessage.m_sBlock.assign(szBlock);
  errorMessage.m_sFile.assign(szFile);
  errorMessage.m_iLine = iLine;
  errorMessage.m_sFunction.assign(szFunction);
  errorMessage.m_sMessage.assign(szMsg);
}

void nsTestFrameworkResult::TestResult(nsUInt32 uiTestIndex, nsUInt32 uiSubTestIndex, bool bSuccess, double fDuration)
{
  nsTestResultData& Result = (uiSubTestIndex == nsInvalidIndex) ? m_Tests[uiTestIndex].m_Result : m_Tests[uiTestIndex].m_SubTests[uiSubTestIndex].m_Result;

  Result.m_bExecuted = true;
  Result.m_bSuccess = bSuccess;
  Result.m_fTestDuration = fDuration;

  // Accumulate sub-test duration onto test duration to get duration feedback while the sub-tests are running.
  // Final time will be set again once the entire test finishes and currently these times are identical as
  // init and de-init times aren't measured at the moment due to missing timer when engine is shut down.
  if (uiSubTestIndex != nsInvalidIndex)
  {
    m_Tests[uiTestIndex].m_Result.m_fTestDuration += fDuration;
  }
}

void nsTestFrameworkResult::AddAsserts(nsUInt32 uiTestIndex, nsUInt32 uiSubTestIndex, int iCount)
{
  if (uiTestIndex != nsInvalidIndex)
  {
    m_Tests[uiTestIndex].m_Result.m_iTestAsserts += iCount;
  }

  if (uiSubTestIndex != nsInvalidIndex)
  {
    m_Tests[uiTestIndex].m_SubTests[uiSubTestIndex].m_Result.m_iTestAsserts += iCount;
  }
}

void nsTestFrameworkResult::SetCustomStatus(nsUInt32 uiTestIndex, nsUInt32 uiSubTestIndex, const char* szCustomStatus)
{
  if (uiTestIndex != nsInvalidIndex && uiSubTestIndex != nsInvalidIndex)
  {
    m_Tests[uiTestIndex].m_SubTests[uiSubTestIndex].m_Result.m_sCustomStatus = szCustomStatus;
  }
}

nsUInt32 nsTestFrameworkResult::GetOutputMessageCount(nsUInt32 uiTestIndex, nsUInt32 uiSubTestIndex, nsTestOutput::Enum type) const
{
  if (uiTestIndex == nsInvalidIndex && type == nsTestOutput::AllOutputTypes)
    return (nsUInt32)m_TestOutput.size();

  nsInt32 iStartIdx = 0;
  nsInt32 iEndIdx = (nsInt32)m_TestOutput.size() - 1;

  if (uiTestIndex != nsInvalidIndex)
  {
    const nsTestResultData& result = GetTestResultData(uiTestIndex, uiSubTestIndex);
    iStartIdx = result.m_iFirstOutput;
    iEndIdx = result.m_iLastOutput;

    // If no messages have been output (yet) for the given test we early-out here.
    if (iStartIdx == -1)
      return 0;

    // If all message types should be counted we can simply return the range.
    if (type == nsTestOutput::AllOutputTypes)
      return iEndIdx - iStartIdx + 1;
  }

  nsUInt32 uiAccumulator = 0;
  for (nsInt32 uiOutputMessageIdx = iStartIdx; uiOutputMessageIdx <= iEndIdx; ++uiOutputMessageIdx)
  {
    if (m_TestOutput[uiOutputMessageIdx].m_Type == type)
      uiAccumulator++;
  }
  return uiAccumulator;
}

const nsTestOutputMessage* nsTestFrameworkResult::GetOutputMessage(nsUInt32 uiOutputMessageIdx) const
{
  return &m_TestOutput[uiOutputMessageIdx];
}

nsUInt32 nsTestFrameworkResult::GetErrorMessageCount(nsUInt32 uiTestIndex, nsUInt32 uiSubTestIndex) const
{
  // If no test is given we can simply return the total error count.
  if (uiTestIndex == nsInvalidIndex)
  {
    return (nsUInt32)m_Errors.size();
  }

  return GetOutputMessageCount(uiTestIndex, uiSubTestIndex, nsTestOutput::Error);
}

const nsTestErrorMessage* nsTestFrameworkResult::GetErrorMessage(nsUInt32 uiErrorMessageIdx) const
{
  return &m_Errors[uiErrorMessageIdx];
}


////////////////////////////////////////////////////////////////////////
// nsTestFrameworkResult public functions
////////////////////////////////////////////////////////////////////////

void nsTestFrameworkResult::nsTestResult::Reset()
{
  m_Result.Reset();
  const nsUInt32 uiSubTestCount = (nsUInt32)m_SubTests.size();
  for (nsUInt32 uiSubTest = 0; uiSubTest < uiSubTestCount; ++uiSubTest)
  {
    m_SubTests[uiSubTest].m_Result.Reset();
  }
}
