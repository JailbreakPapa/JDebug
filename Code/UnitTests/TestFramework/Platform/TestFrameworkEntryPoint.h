#pragma once

#include <Foundation/Basics.h>

#if NS_ENABLED(NS_PLATFORM_ANDROID)

#  include <TestFramework/Platform/Android/TestFrameworkEntryPoint_android.h>

#else

#  ifdef NS_NV_OPTIMUS
#    undef NS_NV_OPTIMUS
#  endif

#  if NS_ENABLED(NS_PLATFORM_WINDOWS)
#    include <Foundation/Basics/Platform/Win/MinWindows.h>
#    define NS_NV_OPTIMUS                                                                           \
      extern "C"                                                                                    \
      {                                                                                             \
        _declspec(dllexport) nsMinWindows::DWORD NvOptimusEnablement = 0x00000001;                  \
        _declspec(dllexport) nsMinWindows::DWORD AmdPowerXpressRequestHighPerformance = 0x00000001; \
      }
#  else
#    define NS_NV_OPTIMUS
#  endif

/// \brief Macro to define the application entry point for all test applications
#  define NS_TESTFRAMEWORK_ENTRY_POINT_BEGIN(szTestName, szNiceTestName)                    \
    /* Enables that on machines with multiple GPUs the NVIDIA GPU is preferred */           \
    NS_NV_OPTIMUS                                                                           \
    NS_APPLICATION_ENTRY_POINT_CODE_INJECTION                                               \
    int main(int argc, char** argv)                                                         \
    {                                                                                       \
      nsTestSetup::InitTestFramework(szTestName, szNiceTestName, argc, (const char**)argv); \
      /* Execute custom init code here by using the BEGIN/END macros directly */

#  define NS_TESTFRAMEWORK_ENTRY_POINT_END()                        \
    while (nsTestSetup::RunTests() == nsTestAppRun::Continue)       \
    {                                                               \
    }                                                               \
    const nsInt32 iFailedTests = nsTestSetup::GetFailedTestCount(); \
    nsTestSetup::DeInitTestFramework();                             \
    return iFailedTests;                                            \
    }

#endif

#define NS_TESTFRAMEWORK_ENTRY_POINT(szTestName, szNiceTestName)             \
  NS_TESTFRAMEWORK_ENTRY_POINT_BEGIN(szTestName, szNiceTestName)             \
  /* Execute custom init code here by using the BEGIN/END macros directly */ \
  NS_TESTFRAMEWORK_ENTRY_POINT_END()
