#include <Foundation/FoundationPCH.h>

#if WD_ENABLED(WD_PLATFORM_WINDOWS_UWP)
#  include <Foundation/Application/Application.h>
#  include <Foundation/Application/Implementation/uwp/Application_uwp.h>
#  include <Foundation/Basics/Platform/Win/IncludeWindows.h>
#  include <Foundation/IO/OSFile.h>
#  include <Foundation/Strings/StringConversion.h>

// Disable warning produced by CppWinRT
#  pragma warning(disable : 5205)
#  include <winrt/Windows.ApplicationModel.Activation.h>
#  include <winrt/Windows.ApplicationModel.Core.h>
#  include <winrt/Windows.Foundation.Collections.h>
#  include <winrt/Windows.Foundation.h>
#  include <winrt/Windows.UI.Core.h>

using namespace winrt::Windows::ApplicationModel::Core;

wdUwpApplication::wdUwpApplication(wdApplication* application)
  : m_application(application)
{
}

wdUwpApplication::~wdUwpApplication() {}

winrt::Windows::ApplicationModel::Core::IFrameworkView wdUwpApplication::CreateView()
{
  return this->get_strong().try_as<winrt::Windows::ApplicationModel::Core::IFrameworkView>();
}

void wdUwpApplication::Initialize(winrt::Windows::ApplicationModel::Core::CoreApplicationView const& applicationView)
{
  applicationView.Activated({this, &wdUwpApplication::OnViewActivated});
}

void wdUwpApplication::SetWindow(winrt::Windows::UI::Core::CoreWindow const& window)
{
}

void wdUwpApplication::Load(winrt::hstring const& entryPoint)
{
}

void wdUwpApplication::Run()
{
  if (wdRun_Startup(m_application).Succeeded())
  {
    auto window = winrt::Windows::UI::Core::CoreWindow::GetForCurrentThread();
    window.Activate();

    wdRun_MainLoop(m_application);
  }
  wdRun_Shutdown(m_application);
}

void wdUwpApplication::Uninitialize()
{
}

void wdUwpApplication::OnViewActivated(winrt::Windows::ApplicationModel::Core::CoreApplicationView const& sender, winrt::Windows::ApplicationModel::Activation::IActivatedEventArgs const& args)
{
  sender.Activated(m_activateRegistrationToken);

  if (args.Kind() == winrt::Windows::ApplicationModel::Activation::ActivationKind::Launch)
  {
    auto launchArgs = args.as<winrt::Windows::ApplicationModel::Activation::LaunchActivatedEventArgs>();
    winrt::hstring argHString = launchArgs.Arguments();

    wdDynamicArray<const char*> argv;
    wdCommandLineUtils::SplitCommandLineString(wdStringUtf8(argHString.c_str()).GetData(), true, m_commandLineArgs, argv);

    m_application->SetCommandLineArguments(argv.GetCount(), argv.GetData());
  }
}

WD_FOUNDATION_DLL wdResult wdUWPRun(wdApplication* pApp)
{
  {
    auto application = winrt::make<wdUwpApplication>(pApp);
    winrt::Windows::ApplicationModel::Core::CoreApplication::Run(application.as<winrt::Windows::ApplicationModel::Core::IFrameworkViewSource>());
  }

  return WD_SUCCESS;
}

#endif

WD_STATICLINK_FILE(Foundation, Foundation_Application_Implementation_uwp_Application_uwp);
