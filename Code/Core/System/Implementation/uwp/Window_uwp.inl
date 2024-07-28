#include <Core/CorePCH.h>

#include <Core/System/Window.h>
#include <Foundation/Basics.h>
#include <Foundation/Basics/Platform/uwp/UWPUtils.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Types/UniquePtr.h>
#include <windows.applicationmodel.core.h>
#include <windows.ui.core.h>

namespace
{
  struct nsWindowUwpData
  {
    ComPtr<ABI::Windows::UI::Core::ICoreDispatcher> m_dispatcher;
    ComPtr<ABI::Windows::UI::Core::ICoreWindow> m_coreWindow;
  };
  nsUniquePtr<nsWindowUwpData> s_uwpWindowData;
} // namespace

nsResult nsWindow::Initialize()
{
  NS_LOG_BLOCK("nsWindow::Initialize", m_CreationDescription.m_Title.GetData());

  if (m_bInitialized)
  {
    Destroy().IgnoreResult();
  }

  // Checking and adjustments to creation desc.
  {
    if (m_CreationDescription.m_WindowMode == nsWindowMode::FullscreenFixedResolution)
    {
      nsLog::Warning("nsWindowMode::FullscreenFixedResolution is not supported on UWP. Falling back to "
                     "nsWindowMode::FullscreenBorderlessNativeResolution.");
      m_CreationDescription.m_WindowMode = nsWindowMode::FullscreenBorderlessNativeResolution;
    }
    else if (m_CreationDescription.m_WindowMode == nsWindowMode::WindowFixedResolution)
    {
      nsLog::Warning("nsWindowMode::WindowFixedResolution is not supported on UWP since resizing a window can not be restricted. Falling "
                     "back to nsWindowMode::WindowResizable");
      m_CreationDescription.m_WindowMode = nsWindowMode::WindowResizable;
    }

    if (m_CreationDescription.AdjustWindowSizeAndPosition().Failed())
      nsLog::Warning("Failed to adjust window size and position settings.");

    NS_ASSERT_RELEASE(m_CreationDescription.m_Resolution.HasNonZeroArea(), "The client area size can't be zero sized!");
  }

  // The main window is handled in a special way in UWP (closing it closes the application, not created explicitely, every window has a
  // thread, ...) which is why we're supporting only a single window for for now.
  NS_ASSERT_RELEASE(s_uwpWindowData == nullptr, "Currently, there is only a single UWP window supported!");


  s_uwpWindowData = NS_DEFAULT_NEW(nsWindowUwpData);

  ComPtr<ABI::Windows::ApplicationModel::Core::ICoreImmersiveApplication> application;
  NS_HRESULT_TO_FAILURE(ABI::Windows::Foundation::GetActivationFactory(
    HStringReference(RuntimeClass_Windows_ApplicationModel_Core_CoreApplication).Get(), &application));

  ComPtr<ABI::Windows::ApplicationModel::Core::ICoreApplicationView> mainView;
  NS_HRESULT_TO_FAILURE(application->get_MainView(&mainView));

  NS_HRESULT_TO_FAILURE(mainView->get_CoreWindow(&s_uwpWindowData->m_coreWindow));
  m_hWindowHandle = s_uwpWindowData->m_coreWindow.Get();

  // Activation of main window already done in Uwp application implementation.
  //  NS_HRESULT_TO_FAILURE(s_uwpWindowData->m_coreWindow->Activate());
  NS_HRESULT_TO_FAILURE(s_uwpWindowData->m_coreWindow->get_Dispatcher(&s_uwpWindowData->m_dispatcher));

  {
    // Get current *logical* screen DPI to do a pixel correct resize.
    ComPtr<ABI::Windows::Graphics::Display::IDisplayInformationStatics> displayInfoStatics;
    NS_HRESULT_TO_FAILURE(ABI::Windows::Foundation::GetActivationFactory(
      HStringReference(RuntimeClass_Windows_Graphics_Display_DisplayInformation).Get(), &displayInfoStatics));
    ComPtr<ABI::Windows::Graphics::Display::IDisplayInformation> displayInfo;
    NS_HRESULT_TO_FAILURE(displayInfoStatics->GetForCurrentView(&displayInfo));
    FLOAT logicalDpi = 1.0f;
    NS_HRESULT_TO_FAILURE(displayInfo->get_LogicalDpi(&logicalDpi));

    // Need application view for the next steps...
    ComPtr<ABI::Windows::UI::ViewManagement::IApplicationViewStatics2> appViewStatics;
    NS_HRESULT_TO_FAILURE(ABI::Windows::Foundation::GetActivationFactory(
      HStringReference(RuntimeClass_Windows_UI_ViewManagement_ApplicationView).Get(), &appViewStatics));
    ComPtr<ABI::Windows::UI::ViewManagement::IApplicationView> appView;
    NS_HRESULT_TO_FAILURE(appViewStatics->GetForCurrentView(&appView));
    ComPtr<ABI::Windows::UI::ViewManagement::IApplicationView2> appView2;
    NS_HRESULT_TO_FAILURE(appView.As(&appView2));
    ComPtr<ABI::Windows::UI::ViewManagement::IApplicationView3> appView3;
    NS_HRESULT_TO_FAILURE(appView.As(&appView3));

    // Request/remove fullscreen from window if requested.
    boolean isFullscreen;
    NS_HRESULT_TO_FAILURE(appView3->get_IsFullScreenMode(&isFullscreen));
    if ((isFullscreen > 0) != nsWindowMode::IsFullscreen(m_CreationDescription.m_WindowMode))
    {
      if (nsWindowMode::IsFullscreen(m_CreationDescription.m_WindowMode))
      {
        NS_HRESULT_TO_FAILURE(appView3->TryEnterFullScreenMode(&isFullscreen));
        if (!isFullscreen)
          nsLog::Warning("Failed to enter full screen mode.");
      }
      else
      {
        NS_HRESULT_TO_FAILURE(appView3->ExitFullScreenMode());
      }
    }

    // Set size. Pointless though if we're fullscreen.
    if (!isFullscreen)
    {
      boolean successfulResize = false;
      ABI::Windows::Foundation::Size size;
      size.Width = m_CreationDescription.m_Resolution.width * 96.0f / logicalDpi;
      size.Height = m_CreationDescription.m_Resolution.height * 96.0f / logicalDpi;
      NS_HRESULT_TO_FAILURE(appView3->TryResizeView(size, &successfulResize));
      if (!successfulResize)
      {
        ABI::Windows::Foundation::Rect visibleBounds;
        NS_HRESULT_TO_FAILURE(appView2->get_VisibleBounds(&visibleBounds));
        nsUInt32 actualWidth = static_cast<nsUInt32>(visibleBounds.Width * (logicalDpi / 96.0f));
        nsUInt32 actualHeight = static_cast<nsUInt32>(visibleBounds.Height * (logicalDpi / 96.0f));

        nsLog::Warning("Failed to resize the window to {0}x{1}, instead (visible) size remains at {2}x{3}",
          m_CreationDescription.m_Resolution.width, m_CreationDescription.m_Resolution.height, actualWidth, actualHeight);

        // m_CreationDescription.m_Resolution.width = actualWidth;
        // m_CreationDescription.m_Resolution.height = actualHeight;
      }
    }
  }

  m_pInputDevice = NS_DEFAULT_NEW(nsStandardInputDevice, s_uwpWindowData->m_coreWindow.Get());
  m_pInputDevice->SetClipMouseCursor(m_CreationDescription.m_bClipMouseCursor ? nsMouseCursorClipMode::ClipToWindowImmediate : nsMouseCursorClipMode::NoClip);
  m_pInputDevice->SetShowMouseCursor(m_CreationDescription.m_bShowMouseCursor);

  m_bInitialized = true;

  return NS_SUCCESS;
}

nsResult nsWindow::Destroy()
{
  if (!m_bInitialized)
    return NS_SUCCESS;

  NS_LOG_BLOCK("nsWindow::Destroy");

  m_pInputDevice = nullptr;
  s_uwpWindowData = nullptr;


  nsLog::Success("Window destroyed.");

  return NS_SUCCESS;
}

nsResult nsWindow::Resize(const nsSizeU32& newWindowSize)
{
  // #TODO Resizing fails on UWP already via the init code.
  return NS_FAILURE;
}

void nsWindow::ProcessWindowMessages()
{
  NS_ASSERT_RELEASE(s_uwpWindowData != nullptr, "No uwp window data available.");

  // Apparently ProcessAllIfPresent does NOT process all events in the queue somehow
  // if this isn't executed quite often every frame (even at 60 Hz), spatial input events quickly queue up
  // and are delivered with many seconds delay
  // as far as I can tell, there is no way to figure out whether there are more queued events
  for (int i = 0; i < 64; ++i)
  {
    HRESULT result = s_uwpWindowData->m_dispatcher->ProcessEvents(ABI::Windows::UI::Core::CoreProcessEventsOption_ProcessAllIfPresent);
    if (FAILED(result))
    {
      nsLog::Error("Window event processing failed with error code: {0}", result);
    }
  }
}

void nsWindow::OnResize(const nsSizeU32& newWindowSize)
{
  nsLog::Info("Window resized to ({0}, {1})", newWindowSize.width, newWindowSize.height);
}

nsWindowHandle nsWindow::GetNativeWindowHandle() const
{
  return m_hWindowHandle;
}
