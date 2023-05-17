#pragma once

#include <Foundation/FoundationInternal.h>
WD_FOUNDATION_INTERNAL_HEADER

#if WD_ENABLED(WD_PLATFORM_WINDOWS_UWP)

#  include <Foundation/Basics.h>
#  include <Foundation/Strings/String.h>

#  include <Foundation/Basics/Platform/uwp/UWPUtils.h>
#  include <winrt/base.h>

#  include <winrt/Windows.ApplicationModel.Activation.h>
#  include <winrt/Windows.ApplicationModel.Core.h>

class wdApplication;

/// Minimal implementation of a uwp application.
class wdUwpApplication : public winrt::implements<wdUwpApplication, winrt::Windows::ApplicationModel::Core::IFrameworkView, winrt::Windows::ApplicationModel::Core::IFrameworkViewSource>
{
public:
  wdUwpApplication(wdApplication* application);
  virtual ~wdUwpApplication();

  // Inherited via IFrameworkViewSource
  winrt::Windows::ApplicationModel::Core::IFrameworkView CreateView();

  // Inherited via IFrameworkView
  void Initialize(winrt::Windows::ApplicationModel::Core::CoreApplicationView const& applicationView);
  void SetWindow(winrt::Windows::UI::Core::CoreWindow const& window);
  void Load(winrt::hstring const& entryPoint);
  void Run();
  void Uninitialize();

private:
  // Application lifecycle event handlers.
  void OnViewActivated(winrt::Windows::ApplicationModel::Core::CoreApplicationView const& sender, winrt::Windows::ApplicationModel::Activation::IActivatedEventArgs const& args);

  winrt::event_token m_activateRegistrationToken;

  wdApplication* m_application;
  wdDynamicArray<wdString> m_commandLineArgs;
};

#endif
