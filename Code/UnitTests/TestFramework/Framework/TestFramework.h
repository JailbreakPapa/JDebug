#pragma once

#include <TestFramework/Framework/Declarations.h>
#include <TestFramework/Framework/SimpleTest.h>
#include <TestFramework/Framework/TestBaseClass.h>
#include <TestFramework/Framework/TestResults.h>
#include <TestFramework/Platform/TestFrameworkEntryPoint.h>
#include <TestFramework/TestFrameworkDLL.h>

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Containers/HashTable.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Strings/String.h>

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include <utility>

class nsCommandLineUtils;

// Disable C++/CX adds.
#pragma warning(disable : 4447)

class NS_TEST_DLL nsTestFramework
{
public:
  nsTestFramework(const char* szTestName, const char* szAbsTestOutputDir, const char* szRelTestDataDir, int iArgc, const char** pArgv);
  virtual ~nsTestFramework();

  using OutputHandler = void (*)(nsTestOutput::Enum, const char*);

  // Test management
  void CreateOutputFolder();
  void UpdateReferenceImages();
  const char* GetTestName() const;
  const char* GetAbsOutputPath() const;
  const char* GetRelTestDataPath() const;
  const char* GetAbsTestOrderFilePath() const;
  const char* GetAbsTestSettingsFilePath() const;
  void RegisterOutputHandler(OutputHandler handler);
  void GatherAllTests();
  void LoadTestOrder();
  void ApplyTestOrderFromCommandLine(const nsCommandLineUtils& cmd);
  void LoadTestSettings();
  void AutoSaveTestOrder();
  void SaveTestOrder(const char* const szFilePath);
  void SaveTestSettings(const char* const szFilePath);
  void SetAllTestsEnabledStatus(bool bEnable);
  void SetAllFailedTestsEnabledStatus();
  // Each function on a test must not take longer than the given time or the test process will be terminated.
  void SetTestTimeout(nsUInt32 uiTestTimeoutMS);
  nsUInt32 GetTestTimeout() const;
  void GetTestSettingsFromCommandLine(const nsCommandLineUtils& cmd);

  // Test execution
  void ResetTests();
  nsTestAppRun RunTestExecutionLoop();

  void StartTests();
  void ExecuteNextTest();
  void EndTests();
  void AbortTests();

  // Test queries
  nsUInt32 GetTestCount() const;
  nsUInt32 GetTestEnabledCount() const;
  nsUInt32 GetSubTestEnabledCount(nsUInt32 uiTestIndex) const;
  const std::string& IsTestAvailable(nsUInt32 uiTestIndex) const;
  bool IsTestEnabled(nsUInt32 uiTestIndex) const;
  bool IsSubTestEnabled(nsUInt32 uiTestIndex, nsUInt32 uiSubTestIndex) const;
  void SetTestEnabled(nsUInt32 uiTestIndex, bool bEnabled);
  void SetSubTestEnabled(nsUInt32 uiTestIndex, nsUInt32 uiSubTestIndex, bool bEnabled);

  nsUInt32 GetCurrentTestIndex() const { return m_uiCurrentTestIndex; }
  nsUInt32 GetCurrentSubTestIndex() const { return m_uiCurrentSubTestIndex; }
  nsInt32 GetCurrentSubTestIdentifier() const;

  /// \brief Returns the index of the sub-test with the given identifier.
  ///
  /// Only looks at the currently running test, assuming that the identifier is unique among its sub-tests.
  nsUInt32 FindSubTestIndexForSubTestIdentifier(nsInt32 iSubTestIdentifier) const;

  nsTestEntry* GetTest(nsUInt32 uiTestIndex);
  const nsTestEntry* GetTest(nsUInt32 uiTestIndex) const;
  bool GetTestsRunning() const { return m_bTestsRunning; }

  const nsTestEntry* GetCurrentTest() const;
  const nsSubTestEntry* GetCurrentSubTest() const;

  // Global settings
  TestSettings GetSettings() const;
  void SetSettings(const TestSettings& settings);

  // Test results
  nsTestFrameworkResult& GetTestResult();
  nsInt32 GetTotalErrorCount() const;
  nsInt32 GetTestsPassedCount() const;
  nsInt32 GetTestsFailedCount() const;
  double GetTotalTestDuration() const;

  // Image comparison
  void ScheduleImageComparison(nsUInt32 uiImageNumber, nsUInt32 uiMaxError);
  void ScheduleDepthImageComparison(nsUInt32 uiImageNumber, nsUInt32 uiMaxError);
  bool IsImageComparisonScheduled() const { return m_bImageComparisonScheduled; }
  bool IsDepthImageComparisonScheduled() const { return m_bDepthImageComparisonScheduled; }
  void GenerateComparisonImageName(nsUInt32 uiImageNumber, nsStringBuilder& ref_sImgName);
  void GetCurrentComparisonImageName(nsStringBuilder& ref_sImgName);
  void SetImageReferenceFolderName(const char* szFolderName);
  void SetImageReferenceOverrideFolderName(const char* szFolderName);

  /// \brief Writes an Html file that contains test information and an image diff view for failed image comparisons.
  void WriteImageDiffHtml(const char* szFileName, const nsImage& referenceImgRgb, const nsImage& referenceImgAlpha, const nsImage& capturedImgRgb, const nsImage& capturedImgAlpha, const nsImage& diffImgRgb, const nsImage& diffImgAlpha, nsUInt32 uiError, nsUInt32 uiThreshold, nsUInt8 uiMinDiffRgb,
    nsUInt8 uiMaxDiffRgb, nsUInt8 uiMinDiffAlpha, nsUInt8 uiMaxDiffAlpha);

  bool PerformImageComparison(nsStringBuilder sImgName, const nsImage& img, nsUInt32 uiMaxError, bool bIsLineImage, char* szErrorMsg);
  bool CompareImages(nsUInt32 uiImageNumber, nsUInt32 uiMaxError, char* szErrorMsg, bool bIsDepthImage = false, bool bIsLineImage = false);

  /// \brief A function to be called to add extra info to image diff output, that is not available from here.
  /// E.g. device specific info like driver version.
  using ImageDiffExtraInfoCallback = std::function<nsDynamicArray<std::pair<nsString, nsString>>()>;
  void SetImageDiffExtraInfoCallback(ImageDiffExtraInfoCallback provider);

  using ImageComparisonCallback = std::function<void(bool)>; /// \brief A function to be called after every image comparison with a bool
                                                             /// indicating if the images matched or not.
  void SetImageComparisonCallback(const ImageComparisonCallback& callback);

  static nsResult CaptureRegressionStat(nsStringView sTestName, nsStringView sName, nsStringView sUnit, float value, nsInt32 iTestId = -1);

protected:
  void Initialize();
  void DeInitialize();

  /// \brief Will be called for test failures to record the location of the failure and forward the error to OutputImpl.
  virtual void ErrorImpl(const char* szError, const char* szFile, nsInt32 iLine, const char* szFunction, const char* szMsg);
  /// \brief Receives nsLog messages (via LogWriter) as well as test-framework internal logging. Any nsTestOutput::Error will
  /// cause the test to fail.
  virtual void OutputImpl(nsTestOutput::Enum Type, const char* szMsg);
  virtual void TestResultImpl(nsUInt32 uiSubTestIndex, bool bSuccess, double fDuration);
  virtual void SetSubTestStatusImpl(nsUInt32 uiSubTestIndex, const char* szStatus);
  void FlushAsserts();
  void TimeoutThread();
  void UpdateTestTimeout();

  // ignore this for now
public:
  static const char* s_szTestBlockName;
  static int s_iAssertCounter;
  static bool s_bCallstackOnAssert;
  static nsLog::TimestampMode s_LogTimestampMode;

  // static functions
public:
  static NS_ALWAYS_INLINE nsTestFramework* GetInstance() { return s_pInstance; }

  /// \brief Returns whether to assert on test failure.
  static bool GetAssertOnTestFail();

  static void Output(nsTestOutput::Enum type, const char* szMsg, ...);
  static void OutputArgs(nsTestOutput::Enum type, const char* szMsg, va_list szArgs);
  static void Error(const char* szError, const char* szFile, nsInt32 iLine, const char* szFunction, nsStringView sMsg, ...);
  static void Error(const char* szError, const char* szFile, nsInt32 iLine, const char* szFunction, nsStringView sMsg, va_list szArgs);
  static void TestResult(nsUInt32 uiSubTestIndex, bool bSuccess, double fDuration);
  static void SetSubTestStatus(nsUInt32 uiSubTestIndex, const char* szStatus);

  // static members
private:
  static nsTestFramework* s_pInstance;

private:
  std::string m_sTestName;                ///< The name of the tests being done
  std::string m_sAbsTestOutputDir;        ///< Absolute path to the output folder where results and temp data is stored
  std::string m_sRelTestDataDir;          ///< Relative path from the SDK to where the unit test data is located
  std::string m_sAbsTestOrderFilePath;    ///< Absolute path to the test order file
  std::string m_sAbsTestSettingsFilePath; ///< Absolute path to the test settings file
  nsInt32 m_iErrorCount = 0;
  nsInt32 m_iTestsFailed = 0;
  nsInt32 m_iTestsPassed = 0;
  TestSettings m_Settings;
  std::recursive_mutex m_OutputMutex;
  std::deque<OutputHandler> m_OutputHandlers;
  std::deque<nsTestEntry> m_TestEntries;
  nsTestFrameworkResult m_Result;
  nsAssertHandler m_PreviousAssertHandler = nullptr;
  ImageDiffExtraInfoCallback m_ImageDiffExtraInfoCallback;
  ImageComparisonCallback m_ImageComparisonCallback;

  std::mutex m_TimeoutLock;
  nsUInt32 m_uiTimeoutMS = 5 * 60 * 1000; // 5 min default timeout
  bool m_bUseTimeout = false;
  bool m_bArm = false;
  std::condition_variable m_TimeoutCV;
  std::thread m_TimeoutThread;

  nsUInt32 m_uiExecutingTest = 0;
  nsUInt32 m_uiExecutingSubTest = 0;
  bool m_bSubTestInitialized = false;
  bool m_bAbortTests = false;
  nsUInt8 m_uiPassesLeft = 0;
  double m_fTotalTestDuration = 0.0;
  double m_fTotalSubTestDuration = 0.0;
  nsInt32 m_iErrorCountBeforeTest = 0;
  nsUInt32 m_uiSubTestInvocationCount = 0;

  bool m_bIsInitialized = false;

  // image comparisons
  bool m_bImageComparisonScheduled = false;
  nsUInt32 m_uiMaxImageComparisonError = 0;
  nsUInt32 m_uiComparisonImageNumber = 0;

  bool m_bDepthImageComparisonScheduled = false;
  nsUInt32 m_uiMaxDepthImageComparisonError = 0;
  nsUInt32 m_uiComparisonDepthImageNumber = 0;

  std::string m_sImageReferenceFolderName = "Images_Reference";
  std::string m_sImageReferenceOverrideFolderName;

protected:
  nsUInt32 m_uiCurrentTestIndex = nsInvalidIndex;
  nsUInt32 m_uiCurrentSubTestIndex = nsInvalidIndex;
  bool m_bTestsRunning = false;
};

/// \brief Enum for usage in NS_TEST_BLOCK to enable or disable the block.
struct nsTestBlock
{
  /// \brief Enum for usage in NS_TEST_BLOCK to enable or disable the block.
  enum Enum
  {
    Enabled,           ///< The test block is enabled.
    Disabled,          ///< The test block will be skipped. The test framework will print a warning message, that some block is deactivated.
    DisabledNoWarning, ///< The test block will be skipped, but no warning printed. Used to deactivate 'on demand/optional' tests.
  };
};

#define safeprintf nsStringUtils::snprintf

/// \brief Starts a small test block inside a larger test.
///
/// First parameter allows to quickly disable a block depending on a condition (e.g. platform).
/// Second parameter just gives it a name for better error reporting.
/// Also skipped tests are highlighted in the output, such that people can quickly see when a test is currently deactivated.
#define NS_TEST_BLOCK(enable, name)                                                  \
  nsTestFramework::s_szTestBlockName = name;                                         \
  if (enable == nsTestBlock::Disabled)                                               \
  {                                                                                  \
    nsTestFramework::s_szTestBlockName = "";                                         \
    nsTestFramework::Output(nsTestOutput::Warning, "Skipped Test Block '%s'", name); \
  }                                                                                  \
  else if (enable == nsTestBlock::DisabledNoWarning)                                 \
  {                                                                                  \
    nsTestFramework::s_szTestBlockName = "";                                         \
  }                                                                                  \
  else


/// \brief Will trigger a debug break, if the test framework is configured to do so on test failure
#define NS_TEST_DEBUG_BREAK                   \
  if (nsTestFramework::GetAssertOnTestFail()) \
  NS_DEBUG_BREAK

#define NS_TEST_FAILURE(erroroutput, msg, ...)                                                                   \
  {                                                                                                              \
    nsTestFramework::Error(erroroutput, NS_SOURCE_FILE, NS_SOURCE_LINE, NS_SOURCE_FUNCTION, msg, ##__VA_ARGS__); \
    NS_TEST_DEBUG_BREAK                                                                                          \
  }

//////////////////////////////////////////////////////////////////////////

NS_TEST_DLL bool nsTestBool(
  bool bCondition, const char* szErrorText, const char* szFile, nsInt32 iLine, const char* szFunction, const char* szMsg, ...);

/// \brief Tests for a boolean condition, does not output an extra message.
#define NS_TEST_BOOL(condition) NS_TEST_BOOL_MSG(condition, "")

/// \brief Tests for a boolean condition, outputs a custom message on failure.
#define NS_TEST_BOOL_MSG(condition, msg, ...) \
  nsTestBool(condition, "Test failed: " NS_STRINGIZE(condition), NS_SOURCE_FILE, NS_SOURCE_LINE, NS_SOURCE_FUNCTION, msg, ##__VA_ARGS__)

//////////////////////////////////////////////////////////////////////////

NS_TEST_DLL bool nsTestResult(
  nsResult condition, const char* szErrorText, const char* szFile, nsInt32 iLine, const char* szFunction, const char* szMsg, ...);

/// \brief Tests for a boolean condition, does not output an extra message.
#define NS_TEST_RESULT(condition) NS_TEST_RESULT_MSG(condition, "")

/// \brief Tests for a boolean condition, outputs a custom message on failure.
#define NS_TEST_RESULT_MSG(condition, msg, ...) \
  nsTestResult(condition, "Test failed: " NS_STRINGIZE(condition), NS_SOURCE_FILE, NS_SOURCE_LINE, NS_SOURCE_FUNCTION, msg, ##__VA_ARGS__)

//////////////////////////////////////////////////////////////////////////

NS_TEST_DLL bool nsTestResult(
  nsResult condition, const char* szErrorText, const char* szFile, nsInt32 iLine, const char* szFunction, const char* szMsg, ...);

/// \brief Tests for a boolean condition, does not output an extra message.
#define NS_TEST_RESULT(condition) NS_TEST_RESULT_MSG(condition, "")

/// \brief Tests for a boolean condition, outputs a custom message on failure.
#define NS_TEST_RESULT_MSG(condition, msg, ...) \
  nsTestResult(condition, "Test failed: " NS_STRINGIZE(condition), NS_SOURCE_FILE, NS_SOURCE_LINE, NS_SOURCE_FUNCTION, msg, ##__VA_ARGS__)

//////////////////////////////////////////////////////////////////////////

/// \brief Tests for a nsStatus condition, outputs nsStatus message on failure
#define NS_TEST_STATUS(condition)                 \
  auto NS_CONCAT(l_, NS_SOURCE_LINE) = condition; \
  nsTestResult(NS_CONCAT(l_, NS_SOURCE_LINE).m_Result, "Test failed: " NS_STRINGIZE(condition), NS_SOURCE_FILE, NS_SOURCE_LINE, NS_SOURCE_FUNCTION, NS_CONCAT(l_, NS_SOURCE_LINE).m_sMessage)

inline double ToFloat(int f)
{
  return static_cast<double>(f);
}

inline double ToFloat(float f)
{
  return static_cast<double>(f);
}

inline double ToFloat(double f)
{
  return static_cast<double>(f);
}

NS_TEST_DLL bool nsTestDouble(double f1, double f2, double fEps, const char* szF1, const char* szF2, const char* szFile, nsInt32 iLine,
  const char* szFunction, const char* szMsg, ...);

/// \brief Tests two floats for equality, within a given epsilon. On failure both actual and expected values are output.
#define NS_TEST_FLOAT(f1, f2, epsilon) NS_TEST_FLOAT_MSG(f1, f2, epsilon, "")

/// \brief Tests two floats for equality, within a given epsilon. On failure both actual and expected values are output, also a custom
/// message is printed.
#define NS_TEST_FLOAT_MSG(f1, f2, epsilon, msg, ...)                                                                                               \
  nsTestDouble(ToFloat(f1), ToFloat(f2), ToFloat(epsilon), NS_STRINGIZE(f1), NS_STRINGIZE(f2), NS_SOURCE_FILE, NS_SOURCE_LINE, NS_SOURCE_FUNCTION, \
    msg, ##__VA_ARGS__)


//////////////////////////////////////////////////////////////////////////

/// \brief Tests two doubles for equality, within a given epsilon. On failure both actual and expected values are output.
#define NS_TEST_DOUBLE(f1, f2, epsilon) NS_TEST_DOUBLE_MSG(f1, f2, epsilon, "")

/// \brief Tests two doubles for equality, within a given epsilon. On failure both actual and expected values are output, also a custom
/// message is printed.
#define NS_TEST_DOUBLE_MSG(f1, f2, epsilon, msg, ...)                                                                                              \
  nsTestDouble(ToFloat(f1), ToFloat(f2), ToFloat(epsilon), NS_STRINGIZE(f1), NS_STRINGIZE(f2), NS_SOURCE_FILE, NS_SOURCE_LINE, NS_SOURCE_FUNCTION, \
    msg, ##__VA_ARGS__)

//////////////////////////////////////////////////////////////////////////

NS_TEST_DLL bool nsTestInt(
  nsInt64 i1, nsInt64 i2, const char* szI1, const char* szI2, const char* szFile, nsInt32 iLine, const char* szFunction, const char* szMsg, ...);

/// \brief Tests two ints for equality. On failure both actual and expected values are output.
#define NS_TEST_INT(i1, i2) NS_TEST_INT_MSG(i1, i2, "")

/// \brief Tests two ints for equality. On failure both actual and expected values are output, also a custom message is printed.
#define NS_TEST_INT_MSG(i1, i2, msg, ...) \
  nsTestInt(i1, i2, NS_STRINGIZE(i1), NS_STRINGIZE(i2), NS_SOURCE_FILE, NS_SOURCE_LINE, NS_SOURCE_FUNCTION, msg, ##__VA_ARGS__)

//////////////////////////////////////////////////////////////////////////

NS_TEST_DLL bool nsTestString(nsStringView s1, nsStringView s2, const char* szString1, const char* szString2, const char* szFile, nsInt32 iLine, const char* szFunction, const char* szMsg, ...);

/// \brief Tests two strings for equality. On failure both actual and expected values are output.
#define NS_TEST_STRING(i1, i2) NS_TEST_STRING_MSG(i1, i2, "")

/// \brief Tests two strings for equality. On failure both actual and expected values are output, also a custom message is printed.
#define NS_TEST_STRING_MSG(s1, s2, msg, ...)                                                                                                     \
  nsTestString(static_cast<nsStringView>(s1), static_cast<nsStringView>(s2), NS_STRINGIZE(s1), NS_STRINGIZE(s2), NS_SOURCE_FILE, NS_SOURCE_LINE, \
    NS_SOURCE_FUNCTION, msg, ##__VA_ARGS__)

//////////////////////////////////////////////////////////////////////////

NS_TEST_DLL bool nsTestWString(std::wstring s1, std::wstring s2, const char* szString1, const char* szString2, const char* szFile, nsInt32 iLine,
  const char* szFunction, const char* szMsg, ...);

/// \brief Tests two strings for equality. On failure both actual and expected values are output.
#define NS_TEST_WSTRING(i1, i2) NS_TEST_WSTRING_MSG(i1, i2, "")

/// \brief Tests two strings for equality. On failure both actual and expected values are output, also a custom message is printed.
#define NS_TEST_WSTRING_MSG(s1, s2, msg, ...)                                                                                         \
  nsTestWString(static_cast<const wchar_t*>(s1), static_cast<const wchar_t*>(s2), NS_STRINGIZE(s1), NS_STRINGIZE(s2), NS_SOURCE_FILE, \
    NS_SOURCE_LINE, NS_SOURCE_FUNCTION, msg, ##__VA_ARGS__)

//////////////////////////////////////////////////////////////////////////

/// \brief Tests two strings for equality. On failure both actual and expected values are output. Does not embed the original expression to
/// work around issues with the current code page and unicode literals.
#define NS_TEST_STRING_UNICODE(i1, i2) NS_TEST_STRING_UNICODE_MSG(i1, i2, "")

/// \brief Tests two strings for equality. On failure both actual and expected values are output, also a custom message is printed. Does not
/// embed the original expression to work around issues with the current code page and unicode literals.
#define NS_TEST_STRING_UNICODE_MSG(s1, s2, msg, ...) \
  nsTestString(                                      \
    static_cast<const char*>(s1), static_cast<const char*>(s2), "", "", NS_SOURCE_FILE, NS_SOURCE_LINE, NS_SOURCE_FUNCTION, msg, ##__VA_ARGS__)

//////////////////////////////////////////////////////////////////////////

NS_TEST_DLL bool nsTestVector(
  nsVec4d v1, nsVec4d v2, double fEps, const char* szCondition, const char* szFile, nsInt32 iLine, const char* szFunction, const char* szMsg, ...);

/// \brief Tests two nsVec2's for equality, using some epsilon. On failure both actual and expected values are output.
#define NS_TEST_VEC2(i1, i2, epsilon) NS_TEST_VEC2_MSG(i1, i2, epsilon, "")

/// \brief Tests two nsVec2's for equality. On failure both actual and expected values are output, also a custom message is printed.
#define NS_TEST_VEC2_MSG(r1, r2, epsilon, msg, ...)                                                                                \
  nsTestVector(nsVec4d(ToFloat((r1).x), ToFloat((r1).y), 0, 0), nsVec4d(ToFloat((r2).x), ToFloat((r2).y), 0, 0), ToFloat(epsilon), \
    NS_STRINGIZE(r1) " == " NS_STRINGIZE(r2), NS_SOURCE_FILE, NS_SOURCE_LINE, NS_SOURCE_FUNCTION, msg, ##__VA_ARGS__)

//////////////////////////////////////////////////////////////////////////

/// \brief Tests two nsVec3's for equality, using some epsilon. On failure both actual and expected values are output.
#define NS_TEST_VEC3(i1, i2, epsilon) NS_TEST_VEC3_MSG(i1, i2, epsilon, "")

/// \brief Tests two nsVec3's for equality. On failure both actual and expected values are output, also a custom message is printed.
#define NS_TEST_VEC3_MSG(r1, r2, epsilon, msg, ...)                                                                                          \
  nsTestVector(nsVec4d(ToFloat((r1).x), ToFloat((r1).y), ToFloat((r1).z), 0), nsVec4d(ToFloat((r2).x), ToFloat((r2).y), ToFloat((r2).z), 0), \
    ToFloat(epsilon), NS_STRINGIZE(r1) " == " NS_STRINGIZE(r2), NS_SOURCE_FILE, NS_SOURCE_LINE, NS_SOURCE_FUNCTION, msg, ##__VA_ARGS__)

//////////////////////////////////////////////////////////////////////////

/// \brief Tests two nsVec4's for equality, using some epsilon. On failure both actual and expected values are output.
#define NS_TEST_VEC4(i1, i2, epsilon) NS_TEST_VEC4_MSG(i1, i2, epsilon, "")

/// \brief Tests two nsVec4's for equality. On failure both actual and expected values are output, also a custom message is printed.
#define NS_TEST_VEC4_MSG(r1, r2, epsilon, msg, ...)                                                                                          \
  nsTestVector(nsVec4d(ToFloat((r1).x), ToFloat((r1).y), ToFloat((r1).z), ToFloat((r1).w)),                                                  \
    nsVec4d(ToFloat((r2).x), ToFloat((r2).y), ToFloat((r2).z), ToFloat((r2).w)), ToFloat(epsilon), NS_STRINGIZE(r1) " == " NS_STRINGIZE(r2), \
    NS_SOURCE_FILE, NS_SOURCE_LINE, NS_SOURCE_FUNCTION, msg, ##__VA_ARGS__)

//////////////////////////////////////////////////////////////////////////

NS_TEST_DLL bool nsTestFiles(
  const char* szFile1, const char* szFile2, const char* szFile, nsInt32 iLine, const char* szFunction, const char* szMsg, ...);

#define NS_TEST_FILES(szFile1, szFile2, msg, ...) \
  nsTestFiles(szFile1, szFile2, NS_SOURCE_FILE, NS_SOURCE_LINE, NS_SOURCE_FUNCTION, msg, ##__VA_ARGS__)

NS_TEST_DLL bool nsTestTextFiles(
  const char* szFile1, const char* szFile2, const char* szFile, nsInt32 iLine, const char* szFunction, const char* szMsg, ...);

#define NS_TEST_TEXT_FILES(szFile1, szFile2, msg, ...) \
  nsTestTextFiles(szFile1, szFile2, NS_SOURCE_FILE, NS_SOURCE_LINE, NS_SOURCE_FUNCTION, msg, ##__VA_ARGS__)

//////////////////////////////////////////////////////////////////////////

NS_TEST_DLL bool nsTestImage(
  nsUInt32 uiImageNumber, nsUInt32 uiMaxError, bool bIsDepthImage, bool bIsLineImage, const char* szFile, nsInt32 iLine, const char* szFunction, const char* szMsg, ...);

/// \brief Same as NS_TEST_IMAGE_MSG but uses an empty error message.
#define NS_TEST_IMAGE(ImageNumber, MaxError) NS_TEST_IMAGE_MSG(ImageNumber, MaxError, "")

/// \brief Same as NS_TEST_DEPTH_IMAGE_MSG but uses an empty error message.
#define NS_TEST_DEPTH_IMAGE(ImageNumber, MaxError) NS_TEST_DEPTH_IMAGE_MSG(ImageNumber, MaxError, "")

/// \brief Same as NS_TEST_LINE_IMAGE_MSG but uses an empty error message.
#define NS_TEST_LINE_IMAGE(ImageNumber, MaxError) NS_TEST_LINE_IMAGE_MSG(ImageNumber, MaxError, "")

/// \brief Executes an image comparison right now.
///
/// The reference image is read from disk.
/// The path to the reference image is constructed from the test and sub-test name and the 'ImageNumber'.
/// One can, for instance, use the 'invocation count' that is passed to nsTestBaseClass::RunSubTest() as the ImageNumber,
/// but any other integer is fine as well.
///
/// The current image to compare is taken from nsTestBaseClass::GetImage().
/// Rendering tests typically override this function to return the result of the currently rendered frame.
///
/// 'MaxError' specifies the maximum mean-square error that is still considered acceptable
/// between the reference image and the current image.
///
/// Use the * DEPTH * variant if a depth buffer comparison should be requested.
///
/// \note Some tests need to know at the start, whether an image comparison will be done at the end, so they
/// can capture the image first. For such use cases, use NS_SCHEDULE_IMAGE_TEST at the start of a sub-test instead.
#define NS_TEST_IMAGE_MSG(ImageNumber, MaxError, msg, ...) \
  nsTestImage(ImageNumber, MaxError, false, false, NS_SOURCE_FILE, NS_SOURCE_LINE, NS_SOURCE_FUNCTION, msg, ##__VA_ARGS__)

#define NS_TEST_DEPTH_IMAGE_MSG(ImageNumber, MaxError, msg, ...) \
  nsTestImage(ImageNumber, MaxError, true, false, NS_SOURCE_FILE, NS_SOURCE_LINE, NS_SOURCE_FUNCTION, msg, ##__VA_ARGS__)

/// \brief Same as NS_TEST_IMAGE_MSG, but allows for pixels to shift in a 1-pixel radius to account for different line rasterization of GPU vendors.
#define NS_TEST_LINE_IMAGE_MSG(ImageNumber, MaxError, msg, ...) \
  nsTestImage(ImageNumber, MaxError, false, true, NS_SOURCE_FILE, NS_SOURCE_LINE, NS_SOURCE_FUNCTION, msg, ##__VA_ARGS__)

/// \brief Schedules an NS_TEST_IMAGE to be executed after the current sub-test execution finishes.
///
/// Call this at the beginning of a sub-test, to automatically execute an image comparison when it is finished.
/// Calling nsTestFramework::IsImageComparisonScheduled() will now return true.
///
/// To support image comparisons, tests derived from nsTestBaseClass need to provide the current image through nsTestBaseClass::GetImage().
/// To support 'scheduled' image comparisons, the class should poll nsTestFramework::IsImageComparisonScheduled() every step and capture the
/// image when needed.
///
/// Use the * DEPTH * variant if a depth buffer comparison is intended.
///
/// \note Scheduling image comparisons is an optimization to only capture data when necessary, instead of capturing it every single frame.
#define NS_SCHEDULE_IMAGE_TEST(ImageNumber, MaxError) nsTestFramework::GetInstance()->ScheduleImageComparison(ImageNumber, MaxError);

#define NS_SCHEDULE_DEPTH_IMAGE_TEST(ImageNumber, MaxError) nsTestFramework::GetInstance()->ScheduleDepthImageComparison(ImageNumber, MaxError);
