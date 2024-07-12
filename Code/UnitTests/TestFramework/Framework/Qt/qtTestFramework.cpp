#include <TestFramework/TestFrameworkPCH.h>

#ifdef NS_USE_QT
#  include <TestFramework/Framework/Qt/qtTestFramework.h>

////////////////////////////////////////////////////////////////////////
// nsQtTestFramework public functions
////////////////////////////////////////////////////////////////////////

nsQtTestFramework::nsQtTestFramework(const char* szTestName, const char* szAbsTestDir, const char* szRelTestDataDir, int iArgc, const char** pArgv)
  : nsTestFramework(szTestName, szAbsTestDir, szRelTestDataDir, iArgc, pArgv)
{
  Q_INIT_RESOURCE(resources);
  Initialize();
}

nsQtTestFramework::~nsQtTestFramework() = default;


////////////////////////////////////////////////////////////////////////
// nsQtTestFramework protected functions
////////////////////////////////////////////////////////////////////////

void nsQtTestFramework::OutputImpl(nsTestOutput::Enum Type, const char* szMsg)
{
  nsTestFramework::OutputImpl(Type, szMsg);
}

void nsQtTestFramework::TestResultImpl(nsUInt32 uiSubTestIndex, bool bSuccess, double fDuration)
{
  nsTestFramework::TestResultImpl(uiSubTestIndex, bSuccess, fDuration);
  Q_EMIT TestResultReceived(m_uiCurrentTestIndex, uiSubTestIndex);
}


void nsQtTestFramework::SetSubTestStatusImpl(nsUInt32 uiSubTestIndex, const char* szStatus)
{
  nsTestFramework::SetSubTestStatusImpl(uiSubTestIndex, szStatus);
  Q_EMIT TestResultReceived(m_uiCurrentTestIndex, uiSubTestIndex);
}

#endif
