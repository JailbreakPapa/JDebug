#include <TestFramework/TestFrameworkPCH.h>

#if NS_ENABLED(NS_PLATFORM_WINDOWS_UWP)
#  include <Foundation/Basics/Platform/uwp/UWPUtils.h>
#  include <Foundation/Strings/StringConversion.h>
#  include <TestFramework/Framework/uwp/uwpTestApplication.h>
#  include <TestFramework/Framework/uwp/uwpTestFramework.h>
#  include <windows.ui.core.h>
#  include <wrl/event.h>

using namespace ABI::Windows::Foundation;

nsUwpTestApplication::nsUwpTestApplication(nsTestFramework& testFramework)
  : m_testFramework(testFramework)
{
}

nsUwpTestApplication::~nsUwpTestApplication() {}

HRESULT nsUwpTestApplication::CreateView(IFrameworkView** viewProvider)
{
  *viewProvider = this;
  return S_OK;
}

HRESULT nsUwpTestApplication::Initialize(ICoreApplicationView* applicationView)
{
  using OnActivatedHandler =
    __FITypedEventHandler_2_Windows__CApplicationModel__CCore__CCoreApplicationView_Windows__CApplicationModel__CActivation__CIActivatedEventArgs;
  NS_SUCCEED_OR_RETURN(
    applicationView->add_Activated(Callback<OnActivatedHandler>(this, &nsUwpTestApplication::OnActivated).Get(), &m_eventRegistrationOnActivate));



  nsStartup::StartupBaseSystems();

  return S_OK;
}

HRESULT nsUwpTestApplication::SetWindow(ABI::Windows::UI::Core::ICoreWindow* window)
{
  return S_OK;
}

HRESULT nsUwpTestApplication::Load(HSTRING entryPoint)
{
  return S_OK;
}

HRESULT nsUwpTestApplication::Run()
{
  ComPtr<ABI::Windows::UI::Core::ICoreWindowStatic> coreWindowStatics;
  NS_SUCCEED_OR_RETURN(
    ABI::Windows::Foundation::GetActivationFactory(HStringReference(RuntimeClass_Windows_UI_Core_CoreWindow).Get(), &coreWindowStatics));
  ComPtr<ABI::Windows::UI::Core::ICoreWindow> coreWindow;
  NS_SUCCEED_OR_RETURN(coreWindowStatics->GetForCurrentThread(&coreWindow));
  ComPtr<ABI::Windows::UI::Core::ICoreDispatcher> dispatcher;
  NS_SUCCEED_OR_RETURN(coreWindow->get_Dispatcher(&dispatcher));

  while (m_testFramework.RunTestExecutionLoop() == nsTestAppRun::Continue)
  {
    dispatcher->ProcessEvents(ABI::Windows::UI::Core::CoreProcessEventsOption_ProcessAllIfPresent);
  }

  return S_OK;
}

HRESULT nsUwpTestApplication::Uninitialize()
{
  m_testFramework.AbortTests();
  return S_OK;
}

HRESULT nsUwpTestApplication::OnActivated(ICoreApplicationView* applicationView, IActivatedEventArgs* args)
{
  applicationView->remove_Activated(m_eventRegistrationOnActivate);

  ActivationKind activationKind;
  NS_SUCCEED_OR_RETURN(args->get_Kind(&activationKind));

  if (activationKind == ActivationKind_Launch)
  {
    ComPtr<ILaunchActivatedEventArgs> launchArgs;
    NS_SUCCEED_OR_RETURN(args->QueryInterface(launchArgs.GetAddressOf()));

    HString argHString;
    NS_SUCCEED_OR_RETURN(launchArgs->get_Arguments(argHString.GetAddressOf()));

    nsDynamicArray<nsString> commandLineArgs;
    nsDynamicArray<const char*> argv;
    nsCommandLineUtils::SplitCommandLineString(nsStringUtf8(argHString).GetData(), true, commandLineArgs, argv);

    nsCommandLineUtils cmd;
    cmd.SetCommandLine(argv.GetCount(), argv.GetData(), nsCommandLineUtils::PreferOsArgs);

    m_testFramework.GetTestSettingsFromCommandLine(cmd);

    // Setup an extended execution session to prevent app from going to sleep during testing.
    nsUwpUtils::CreateInstance<IExtendedExecutionSession>(
      RuntimeClass_Windows_ApplicationModel_ExtendedExecution_ExtendedExecutionSession, m_extendedExecutionSession);
    NS_ASSERT_DEV(m_extendedExecutionSession, "Failed to create extended session. Can't prevent app from backgrounding during testing.");
    m_extendedExecutionSession->put_Reason(ExtendedExecutionReason::ExtendedExecutionReason_Unspecified);
    nsStringHString desc("Keep Unit Tests Running");
    m_extendedExecutionSession->put_Description(desc.GetData().Get());

    using OnRevokedHandler = __FITypedEventHandler_2_IInspectable_Windows__CApplicationModel__CExtendedExecution__CExtendedExecutionRevokedEventArgs;
    NS_SUCCEED_OR_RETURN(m_extendedExecutionSession->add_Revoked(
      Callback<OnRevokedHandler>(this, &nsUwpTestApplication::OnSessionRevoked).Get(), &m_eventRegistrationOnRevokedSession));

    ComPtr<__FIAsyncOperation_1_Windows__CApplicationModel__CExtendedExecution__CExtendedExecutionResult> pAsyncOp;
    if (SUCCEEDED(m_extendedExecutionSession->RequestExtensionAsync(&pAsyncOp)))
    {
      nsUwpUtils::nsWinRtPutCompleted<ExtendedExecutionResult, ExtendedExecutionResult>(pAsyncOp, [this](const ExtendedExecutionResult& pResult)
        {
        switch (pResult)
        {
          case ExtendedExecutionResult::ExtendedExecutionResult_Allowed:
            nsLog::Info("Extended session is active.");
            break;
          case ExtendedExecutionResult::ExtendedExecutionResult_Denied:
            nsLog::Error("Extended session is denied.");
            break;
        } });
    }
  }

  return S_OK;
}

HRESULT nsUwpTestApplication::OnSessionRevoked(IInspectable* sender, IExtendedExecutionRevokedEventArgs* args)
{
  nsLog::Error("Extended session revoked.");
  return S_OK;
}

#endif
