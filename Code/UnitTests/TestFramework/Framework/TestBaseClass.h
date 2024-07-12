#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Types/Status.h>
#include <Foundation/Utilities/EnumerableClass.h>
#include <TestFramework/Framework/Declarations.h>

struct nsTestConfiguration;
class nsImage;

class NS_TEST_DLL nsTestBaseClass : public nsEnumerable<nsTestBaseClass>
{
  friend class nsTestFramework;

  NS_DECLARE_ENUMERABLE_CLASS(nsTestBaseClass);

public:
  // *** Override these functions to implement the required test functionality ***

  /// Override this function to give the test a proper name.
  virtual const char* GetTestName() const /*override*/ = 0;

  const char* GetSubTestName(nsInt32 iIdentifier) const;

  /// Override this function to add additional information to the test configuration
  virtual void UpdateConfiguration(nsTestConfiguration& ref_config) const /*override*/;

  /// \brief Implement this to add support for image comparisons. See NS_TEST_IMAGE_MSG.
  virtual nsResult GetImage(nsImage& ref_img, const nsSubTestEntry& subTest, nsUInt32 uiImageNumber) { return NS_FAILURE; }

  /// \brief Implement this to add support for depth buffer image comparisons. See NS_TEST_DEPTH_IMAGE_MSG.
  virtual nsResult GetDepthImage(nsImage& ref_img, const nsSubTestEntry& subTest, nsUInt32 uiImageNumber) { return NS_FAILURE; }

  /// \brief Used to map the 'number' for an image comparison, to a string used for finding the comparison image.
  ///
  /// By default image comparison screenshots are called 'TestName_SubTestName_XYZ'
  /// This can be fully overridden to use any other file name.
  /// The location of the comparison images (ie the folder) cannot be specified at the moment.
  virtual void MapImageNumberToString(const char* szTestName, const nsSubTestEntry& subTest, nsUInt32 uiImageNumber, nsStringBuilder& out_sString) const;

protected:
  /// Called at startup to determine if the test can be run. Should return a detailed error message on failure.
  virtual std::string IsTestAvailable() const { return {}; };
  /// Called at startup to setup all tests. Should use 'AddSubTest' to register all the sub-tests to the test framework.
  virtual void SetupSubTests() = 0;
  /// Called to run the test that was registered with the given identifier.
  virtual nsTestAppRun RunSubTest(nsInt32 iIdentifier, nsUInt32 uiInvocationCount) = 0;

  // *** Override these functions to implement optional (de-)initialization ***

  /// Called to initialize the whole test.
  virtual nsResult InitializeTest() { return NS_SUCCESS; }
  /// Called to deinitialize the whole test.
  virtual nsResult DeInitializeTest() { return NS_SUCCESS; }
  /// Called before running a sub-test to do additional initialization specifically for that test.
  virtual nsResult InitializeSubTest(nsInt32 iIdentifier) { return NS_SUCCESS; }
  /// Called after running a sub-test to do additional deinitialization specifically for that test.
  virtual nsResult DeInitializeSubTest(nsInt32 iIdentifier) { return NS_SUCCESS; }


  /// Adds a sub-test to the test suite. The index is used to identify it when running the sub-tests.
  void AddSubTest(const char* szName, nsInt32 iIdentifier);

private:
  struct TestEntry
  {
    const char* m_szName = "";
    nsInt32 m_iIdentifier = -1;
  };

  /// Removes all sub-tests.
  void ClearSubTests();

  // Called by nsTestFramework.
  nsResult DoTestInitialization();
  void DoTestDeInitialization();
  nsResult DoSubTestInitialization(nsInt32 iIdentifier);
  void DoSubTestDeInitialization(nsInt32 iIdentifier);
  nsTestAppRun DoSubTestRun(nsInt32 iIdentifier, double& fDuration, nsUInt32 uiInvocationCount);

  // Finds internal entry index for identifier
  nsInt32 FindEntryForIdentifier(nsInt32 iIdentifier) const;

  std::deque<TestEntry> m_Entries;
};

#define NS_CREATE_TEST(TestClass)
