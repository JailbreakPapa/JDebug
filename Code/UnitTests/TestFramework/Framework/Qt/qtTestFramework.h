#pragma once

#ifdef NS_USE_QT

#  include <QObject>
#  include <TestFramework/Framework/TestFramework.h>
#  include <TestFramework/TestFrameworkDLL.h>

/// \brief Derived nsTestFramework which signals the GUI to update whenever a new tests result comes in.
class NS_TEST_DLL nsQtTestFramework : public QObject, public nsTestFramework
{
  Q_OBJECT
public:
  nsQtTestFramework(const char* szTestName, const char* szAbsTestDir, const char* szRelTestDataDir, int iArgc, const char** pArgv);
  virtual ~nsQtTestFramework();

private:
  nsQtTestFramework(nsQtTestFramework&);
  void operator=(nsQtTestFramework&);

Q_SIGNALS:
  void TestResultReceived(qint32 testIndex, qint32 subTestIndex);

protected:
  virtual void OutputImpl(nsTestOutput::Enum Type, const char* szMsg) override;
  virtual void TestResultImpl(nsUInt32 uiSubTestIndex, bool bSuccess, double fDuration) override;
  virtual void SetSubTestStatusImpl(nsUInt32 uiSubTestIndex, const char* szStatus) override;
};

#endif
