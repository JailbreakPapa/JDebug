#pragma once

#include <TestFramework/Platform/Android/AndroidTestApplication.h>
#include <android/log.h>

#define NS_TESTFRAMEWORK_ENTRY_POINT_BEGIN(szTestName, szNiceTestName)                                                \
  int nsAndroidMain(int argc, char** argv);                                                                           \
  NS_APPLICATION_ENTRY_POINT_CODE_INJECTION                                                                           \
  extern "C" void android_main(struct android_app* app)                                                               \
  {                                                                                                                   \
    nsAndroidTestApplication androidApp(app);                                                                         \
    androidApp.AndroidRun();                                                                                          \
    const nsInt32 iFailedTests = nsTestSetup::GetFailedTestCount();                                                   \
    nsTestSetup::DeInitTestFramework();                                                                               \
    __android_log_print(ANDROID_LOG_ERROR, "nsEngine", "Test framework exited with return code: '%d'", iFailedTests); \
  }                                                                                                                   \
                                                                                                                      \
  int nsAndroidMain(int argc, char** argv)                                                                            \
  {                                                                                                                   \
    nsTestSetup::InitTestFramework(szTestName, szNiceTestName, 0, nullptr);                                           \
    /* Execute custom init code here by using the BEGIN/END macros directly */

#define NS_TESTFRAMEWORK_ENTRY_POINT_END() \
  return 0;                                \
  }
