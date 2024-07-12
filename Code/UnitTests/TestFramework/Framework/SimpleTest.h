#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Utilities/EnumerableClass.h>
#include <TestFramework/Framework/Declarations.h>
#include <TestFramework/Framework/TestBaseClass.h>

class NS_TEST_DLL nsSimpleTestGroup : public nsTestBaseClass
{
public:
  using SimpleTestFunc = void (*)();

  nsSimpleTestGroup(const char* szName)
    : m_szTestName(szName)
  {
  }

  void AddSimpleTest(const char* szName, SimpleTestFunc testFunc);

  virtual const char* GetTestName() const override { return m_szTestName; }

private:
  virtual void SetupSubTests() override;
  virtual nsTestAppRun RunSubTest(nsInt32 iIdentifier, nsUInt32 uiInvocationCount) override;
  virtual nsResult InitializeSubTest(nsInt32 iIdentifier) override;
  virtual nsResult DeInitializeSubTest(nsInt32 iIdentifier) override;

private:
  struct SimpleTestEntry
  {
    const char* m_szName;
    SimpleTestFunc m_Func;
  };

  const char* m_szTestName;
  std::deque<SimpleTestEntry> m_SimpleTests;
};

class NS_TEST_DLL nsRegisterSimpleTestHelper : public nsEnumerable<nsRegisterSimpleTestHelper>
{
  NS_DECLARE_ENUMERABLE_CLASS(nsRegisterSimpleTestHelper);

public:
  nsRegisterSimpleTestHelper(nsSimpleTestGroup* pTestGroup, const char* szTestName, nsSimpleTestGroup::SimpleTestFunc func)
  {
    m_pTestGroup = pTestGroup;
    m_szTestName = szTestName;
    m_Func = func;
  }

  void RegisterTest() { m_pTestGroup->AddSimpleTest(m_szTestName, m_Func); }

private:
  nsSimpleTestGroup* m_pTestGroup;
  const char* m_szTestName;
  nsSimpleTestGroup::SimpleTestFunc m_Func;
};

#define NS_CREATE_SIMPLE_TEST_GROUP(GroupName) nsSimpleTestGroup NS_CONCAT(g_SimpleTestGroup__, GroupName)(NS_STRINGIZE(GroupName));

#define NS_CREATE_SIMPLE_TEST(GroupName, TestName)                                                                       \
  extern nsSimpleTestGroup NS_CONCAT(g_SimpleTestGroup__, GroupName);                                                    \
  static void nsSimpleTestFunction__##GroupName##_##TestName();                                                          \
  nsRegisterSimpleTestHelper nsRegisterSimpleTest__##GroupName##TestName(                                                \
    &NS_CONCAT(g_SimpleTestGroup__, GroupName), NS_STRINGIZE(TestName), nsSimpleTestFunction__##GroupName##_##TestName); \
  static void nsSimpleTestFunction__##GroupName##_##TestName()
