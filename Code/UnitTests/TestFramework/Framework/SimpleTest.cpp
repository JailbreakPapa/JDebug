#include <TestFramework/TestFrameworkPCH.h>

#include <Foundation/Profiling/Profiling.h>
#include <TestFramework/Framework/TestFramework.h>

NS_ENUMERABLE_CLASS_IMPLEMENTATION(nsRegisterSimpleTestHelper);

void nsSimpleTestGroup::AddSimpleTest(const char* szName, SimpleTestFunc testFunc)
{
  SimpleTestEntry e;
  e.m_szName = szName;
  e.m_Func = testFunc;

  for (nsUInt32 i = 0; i < m_SimpleTests.size(); ++i)
  {
    if ((strcmp(m_SimpleTests[i].m_szName, e.m_szName) == 0) && (m_SimpleTests[i].m_Func == e.m_Func))
      return;
  }

  m_SimpleTests.push_back(e);
}

void nsSimpleTestGroup::SetupSubTests()
{
  for (nsUInt32 i = 0; i < m_SimpleTests.size(); ++i)
  {
    AddSubTest(m_SimpleTests[i].m_szName, i);
  }
}

nsTestAppRun nsSimpleTestGroup::RunSubTest(nsInt32 iIdentifier, nsUInt32 uiInvocationCount)
{
  // until the block name is properly set, use the test name instead
  nsTestFramework::s_szTestBlockName = m_SimpleTests[iIdentifier].m_szName;

  NS_PROFILE_SCOPE(m_SimpleTests[iIdentifier].m_szName);
  m_SimpleTests[iIdentifier].m_Func();

  nsTestFramework::s_szTestBlockName = "";
  return nsTestAppRun::Quit;
}

nsResult nsSimpleTestGroup::InitializeSubTest(nsInt32 iIdentifier)
{
  // initialize everything up to 'core'
  nsStartup::StartupCoreSystems();
  return NS_SUCCESS;
}

nsResult nsSimpleTestGroup::DeInitializeSubTest(nsInt32 iIdentifier)
{
  // shut down completely
  nsStartup::ShutdownCoreSystems();
  nsMemoryTracker::DumpMemoryLeaks();
  return NS_SUCCESS;
}
