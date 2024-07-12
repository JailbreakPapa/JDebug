#include <TestFramework/TestFrameworkPCH.h>

#if NS_ENABLED(NS_PLATFORM_WINDOWS_UWP)
#  include <TestFramework/Framework/uwp/uwpTestApplication.h>
#  include <TestFramework/Framework/uwp/uwpTestFramework.h>

#  include <Foundation/Logging/Log.h>

nsUwpTestFramework::nsUwpTestFramework(const char* szTestName, const char* szAbsTestDir, const char* szRelTestDataDir, int argc, const char** argv)
  : nsTestFramework(szTestName, szAbsTestDir, szRelTestDataDir, argc, argv)
{
}

nsUwpTestFramework::~nsUwpTestFramework()
{
  RoUninitialize();
}

void nsUwpTestFramework::Run()
{
  ComPtr<ABI::Windows::ApplicationModel::Core::ICoreApplication> coreApplication;
  HRESULT result = ABI::Windows::Foundation::GetActivationFactory(
    HStringReference(RuntimeClass_Windows_ApplicationModel_Core_CoreApplication).Get(), &coreApplication);
  if (FAILED(result))
  {
    std::cout << "Failed to create core application." << std::endl;
    return;
  }
  else
  {
    ComPtr<nsUwpTestApplication> application = Make<nsUwpTestApplication>(*this);
    coreApplication->Run(application.Get());
    application.Detach(); // Was already deleted by uwp.
  }
}

#endif
