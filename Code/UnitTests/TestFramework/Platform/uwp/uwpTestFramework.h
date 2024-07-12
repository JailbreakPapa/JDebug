#pragma once

#if NS_ENABLED(NS_PLATFORM_WINDOWS_UWP)

#  include <TestFramework/Framework/TestFramework.h>
#  include <TestFramework/TestFrameworkDLL.h>

#  include <Foundation/Basics/Platform/uwp/UWPUtils.h>

/// \brief Derived nsTestFramework which signals the GUI to update whenever a new tests result comes in.
class NS_TEST_DLL nsUwpTestFramework : public nsTestFramework
{
public:
  nsUwpTestFramework(const char* szTestName, const char* szAbsTestDir, const char* szRelTestDataDir, int argc, const char** argv);
  virtual ~nsUwpTestFramework();

  nsUwpTestFramework(nsUwpTestFramework&) = delete;
  void operator=(nsUwpTestFramework&) = delete;

  void Run();
};

#endif
