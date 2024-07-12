#pragma once

#include <TestFramework/TestFrameworkDLL.h>

class nsTestFramework;

/// \brief A collection of static helper functions to setup the test framework.
class NS_TEST_DLL nsTestSetup
{
public:
  /// \brief Creates and returns a test framework with the given name.
  static nsTestFramework* InitTestFramework(const char* szTestName, const char* szNiceTestName, int iArgc, const char** pArgv);

  /// \brief Runs tests and returns number of errors.
  static nsTestAppRun RunTests();

  static nsInt32 GetFailedTestCount();

  /// \brief Deletes the test framework and outputs final test output.
  ///
  /// If bSilent is true, the function will not print anything to the console (debug info)
  static void DeInitTestFramework(bool bSilent = false);

private:
  static int s_iArgc;
  static const char** s_pArgv;
};
