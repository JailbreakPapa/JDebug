#include <Core/CorePCH.h>

#include <Core/System/Window.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/IO/OpenDdlReader.h>
#include <Foundation/IO/OpenDdlUtils.h>
#include <Foundation/IO/OpenDdlWriter.h>
#include <Foundation/System/Screen.h>

#if NS_ENABLED(NS_SUPPORTS_GLFW)
#  include <Core/System/Implementation/glfw/InputDevice_glfw.inl>
#  include <Core/System/Implementation/glfw/Window_glfw.inl>
#elif NS_ENABLED(NS_PLATFORM_WINDOWS_DESKTOP)
#  include <Core/System/Implementation/Win/InputDevice_win32.inl>
#  include <Core/System/Implementation/Win/Window_win32.inl>
#elif NS_ENABLED(NS_PLATFORM_WINDOWS_UWP)
#  include <Core/System/Implementation/uwp/InputDevice_uwp.inl>
#  include <Core/System/Implementation/uwp/Window_uwp.inl>
#elif NS_ENABLED(NS_PLATFORM_ANDROID)
#  include <Core/System/Implementation/android/InputDevice_android.inl>
#  include <Core/System/Implementation/android/Window_android.inl>
#else
#  include <Core/System/Implementation/null/InputDevice_null.inl>
#  include <Core/System/Implementation/null/Window_null.inl>
#endif

nsUInt8 nsWindow::s_uiNextUnusedWindowNumber = 0;

nsResult nsWindowCreationDesc::AdjustWindowSizeAndPosition()
{
  nsHybridArray<nsScreenInfo, 2> screens;
  if (nsScreen::EnumerateScreens(screens).Failed() || screens.IsEmpty())
    return NS_FAILURE;

  nsInt32 iShowOnMonitor = m_iMonitor;

  if (iShowOnMonitor >= (nsInt32)screens.GetCount())
    iShowOnMonitor = -1;

  const nsScreenInfo* pScreen = nullptr;

  // this means 'pick the primary screen'
  if (iShowOnMonitor < 0)
  {
    pScreen = &screens[0];

    for (nsUInt32 i = 0; i < screens.GetCount(); ++i)
    {
      if (screens[i].m_bIsPrimary)
      {
        pScreen = &screens[i];
        break;
      }
    }
  }
  else
  {
    pScreen = &screens[iShowOnMonitor];
  }

  if (m_WindowMode == nsWindowMode::FullscreenBorderlessNativeResolution)
  {
    m_Resolution.width = pScreen->m_iResolutionX;
    m_Resolution.height = pScreen->m_iResolutionY;
  }
  else
  {
    // clamp the resolution to the native resolution ?
    // m_ClientAreaSize.width = nsMath::Min<nsUInt32>(m_ClientAreaSize.width, pScreen->m_iResolutionX);
    // m_ClientAreaSize.height= nsMath::Min<nsUInt32>(m_ClientAreaSize.height,pScreen->m_iResolutionY);
  }

  if (m_bCenterWindowOnDisplay)
  {
    m_Position.Set(pScreen->m_iOffsetX + (pScreen->m_iResolutionX - (nsInt32)m_Resolution.width) / 2, pScreen->m_iOffsetY + (pScreen->m_iResolutionY - (nsInt32)m_Resolution.height) / 2);
  }
  else
  {
    m_Position.Set(pScreen->m_iOffsetX, pScreen->m_iOffsetY);
  }

  return NS_SUCCESS;
}

void nsWindowCreationDesc::SaveToDDL(nsOpenDdlWriter& ref_writer)
{
  ref_writer.BeginObject("WindowDesc");

  nsOpenDdlUtils::StoreString(ref_writer, m_Title, "Title");

  switch (m_WindowMode.GetValue())
  {
    case nsWindowMode::FullscreenBorderlessNativeResolution:
      nsOpenDdlUtils::StoreString(ref_writer, "Borderless", "Mode");
      break;
    case nsWindowMode::FullscreenFixedResolution:
      nsOpenDdlUtils::StoreString(ref_writer, "Fullscreen", "Mode");
      break;
    case nsWindowMode::WindowFixedResolution:
      nsOpenDdlUtils::StoreString(ref_writer, "Window", "Mode");
      break;
    case nsWindowMode::WindowResizable:
      nsOpenDdlUtils::StoreString(ref_writer, "ResizableWindow", "Mode");
      break;
  }

  if (m_uiWindowNumber != 0)
    nsOpenDdlUtils::StoreUInt8(ref_writer, m_uiWindowNumber, "Index");

  if (m_iMonitor >= 0)
    nsOpenDdlUtils::StoreInt8(ref_writer, m_iMonitor, "Monitor");

  if (m_Position != nsVec2I32(0x80000000, 0x80000000))
  {
    nsOpenDdlUtils::StoreVec2I(ref_writer, m_Position, "Position");
  }

  nsOpenDdlUtils::StoreVec2U(ref_writer, nsVec2U32(m_Resolution.width, m_Resolution.height), "Resolution");

  nsOpenDdlUtils::StoreBool(ref_writer, m_bClipMouseCursor, "ClipMouseCursor");
  nsOpenDdlUtils::StoreBool(ref_writer, m_bShowMouseCursor, "ShowMouseCursor");
  nsOpenDdlUtils::StoreBool(ref_writer, m_bSetForegroundOnInit, "SetForegroundOnInit");
  nsOpenDdlUtils::StoreBool(ref_writer, m_bCenterWindowOnDisplay, "CenterWindowOnDisplay");

  ref_writer.EndObject();
}


nsResult nsWindowCreationDesc::SaveToDDL(nsStringView sFile)
{
  nsFileWriter file;
  NS_SUCCEED_OR_RETURN(file.Open(sFile));

  nsOpenDdlWriter writer;
  writer.SetOutputStream(&file);

  SaveToDDL(writer);

  return NS_SUCCESS;
}

void nsWindowCreationDesc::LoadFromDDL(const nsOpenDdlReaderElement* pParentElement)
{
  if (const nsOpenDdlReaderElement* pDesc = pParentElement->FindChildOfType("WindowDesc"))
  {
    if (const nsOpenDdlReaderElement* pTitle = pDesc->FindChildOfType(nsOpenDdlPrimitiveType::String, "Title"))
      m_Title = pTitle->GetPrimitivesString()[0];

    if (const nsOpenDdlReaderElement* pMode = pDesc->FindChildOfType(nsOpenDdlPrimitiveType::String, "Mode"))
    {
      auto mode = pMode->GetPrimitivesString()[0];

      if (mode == "Borderless")
        m_WindowMode = nsWindowMode::FullscreenBorderlessNativeResolution;
      else if (mode == "Fullscreen")
        m_WindowMode = nsWindowMode::FullscreenFixedResolution;
      else if (mode == "Window")
        m_WindowMode = nsWindowMode::WindowFixedResolution;
      else if (mode == "ResizableWindow")
        m_WindowMode = nsWindowMode::WindowResizable;
    }

    if (const nsOpenDdlReaderElement* pIndex = pDesc->FindChildOfType(nsOpenDdlPrimitiveType::UInt8, "Index"))
    {
      m_uiWindowNumber = pIndex->GetPrimitivesUInt8()[0];
    }

    if (const nsOpenDdlReaderElement* pMonitor = pDesc->FindChildOfType(nsOpenDdlPrimitiveType::Int8, "Monitor"))
    {
      m_iMonitor = pMonitor->GetPrimitivesInt8()[0];
    }

    if (const nsOpenDdlReaderElement* pPosition = pDesc->FindChild("Position"))
    {
      nsOpenDdlUtils::ConvertToVec2I(pPosition, m_Position).IgnoreResult();
    }

    if (const nsOpenDdlReaderElement* pPosition = pDesc->FindChild("Resolution"))
    {
      nsVec2U32 res;
      nsOpenDdlUtils::ConvertToVec2U(pPosition, res).IgnoreResult();
      m_Resolution.width = res.x;
      m_Resolution.height = res.y;
    }

    if (const nsOpenDdlReaderElement* pClipMouseCursor = pDesc->FindChildOfType(nsOpenDdlPrimitiveType::Bool, "ClipMouseCursor"))
      m_bClipMouseCursor = pClipMouseCursor->GetPrimitivesBool()[0];

    if (const nsOpenDdlReaderElement* pShowMouseCursor = pDesc->FindChildOfType(nsOpenDdlPrimitiveType::Bool, "ShowMouseCursor"))
      m_bShowMouseCursor = pShowMouseCursor->GetPrimitivesBool()[0];

    if (const nsOpenDdlReaderElement* pSetForegroundOnInit = pDesc->FindChildOfType(nsOpenDdlPrimitiveType::Bool, "SetForegroundOnInit"))
      m_bSetForegroundOnInit = pSetForegroundOnInit->GetPrimitivesBool()[0];

    if (const nsOpenDdlReaderElement* pCenterWindowOnDisplay = pDesc->FindChildOfType(nsOpenDdlPrimitiveType::Bool, "CenterWindowOnDisplay"))
      m_bCenterWindowOnDisplay = pCenterWindowOnDisplay->GetPrimitivesBool()[0];
  }
}

nsResult nsWindowCreationDesc::LoadFromDDL(nsStringView sFile)
{
  nsFileReader file;
  NS_SUCCEED_OR_RETURN(file.Open(sFile));

  nsOpenDdlReader reader;
  NS_SUCCEED_OR_RETURN(reader.ParseDocument(file));

  LoadFromDDL(reader.GetRootElement());

  return NS_SUCCESS;
}

nsWindow::nsWindow()
{
  ++s_uiNextUnusedWindowNumber;
}

nsWindow::~nsWindow()
{
  if (m_bInitialized)
  {
    Destroy().IgnoreResult();
  }
  NS_ASSERT_DEV(m_iReferenceCount == 0, "The window is still being referenced, probably by a swapchain. Make sure to destroy all swapchains and call nsGALDevice::WaitIdle before destroying a window.");
}

#if NS_ENABLED(NS_PLATFORM_WINDOWS)
void nsWindow::OnWindowMessage(nsMinWindows::HWND hWnd, nsMinWindows::UINT msg, nsMinWindows::WPARAM wparam, nsMinWindows::LPARAM lparam)
{
}
#endif

nsUInt8 nsWindow::GetNextUnusedWindowNumber()
{
  return s_uiNextUnusedWindowNumber;
}
