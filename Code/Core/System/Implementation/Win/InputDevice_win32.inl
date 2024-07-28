#include <Core/Input/InputManager.h>
#include <Core/System/Implementation/Win/InputDevice_win32.h>
#include <Foundation/Basics/Platform/Win/IncludeWindows.h>
#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Strings/StringConversion.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsStandardInputDevice, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

bool nsStandardInputDevice::s_bMainWindowUsed = false;

nsStandardInputDevice::nsStandardInputDevice(nsUInt32 uiWindowNumber)
{
  m_uiWindowNumber = uiWindowNumber;

  if (uiWindowNumber == 0)
  {
    NS_ASSERT_RELEASE(!s_bMainWindowUsed, "You cannot have two devices of Type nsStandardInputDevice with the window number zero.");
    nsStandardInputDevice::s_bMainWindowUsed = true;
  }

  m_DoubleClickTime = nsTime::MakeFromMilliseconds(GetDoubleClickTime());
}

nsStandardInputDevice::~nsStandardInputDevice()
{
  if (!m_bShowCursor)
  {
    ShowCursor(true);
  }

  if (m_uiWindowNumber == 0)
    nsStandardInputDevice::s_bMainWindowUsed = false;
}

void nsStandardInputDevice::InitializeDevice()
{
  if (m_uiWindowNumber == 0)
  {
    RAWINPUTDEVICE Rid[2];

    // keyboard
    Rid[0].usUsagePage = 0x01;
    Rid[0].usUsage = 0x06;
    Rid[0].dwFlags = RIDEV_NOHOTKEYS; // Disables Windows-Key and Application-Key
    Rid[0].hwndTarget = nullptr;

    // mouse
    Rid[1].usUsagePage = 0x01;
    Rid[1].usUsage = 0x02;
    Rid[1].dwFlags = 0;
    Rid[1].hwndTarget = nullptr;

    if (RegisterRawInputDevices(&Rid[0], (UINT)2, sizeof(RAWINPUTDEVICE)) == FALSE)
    {
      nsLog::Error("Could not initialize RawInput for Mouse and Keyboard input.");
    }
    else
      nsLog::Success("Initialized RawInput for Mouse and Keyboard input.");
  }
  else
    nsLog::Info("Window {0} does not need to initialize Mouse or Keyboard.", m_uiWindowNumber);
}

void nsStandardInputDevice::RegisterInputSlots()
{
  RegisterInputSlot(nsInputSlot_KeyLeft, "Left", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyRight, "Right", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyUp, "Up", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyDown, "Down", nsInputSlotFlags::IsButton);

  RegisterInputSlot(nsInputSlot_KeyEscape, "Escape", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeySpace, "Space", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyBackspace, "Backspace", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyReturn, "Return", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyTab, "Tab", nsInputSlotFlags::IsButton);

  RegisterInputSlot(nsInputSlot_KeyLeftShift, "Left Shift", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyRightShift, "Right Shift", nsInputSlotFlags::IsButton);

  RegisterInputSlot(nsInputSlot_KeyLeftCtrl, "Left Ctrl", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyRightCtrl, "Right Ctrl", nsInputSlotFlags::IsButton);

  RegisterInputSlot(nsInputSlot_KeyLeftAlt, "Left Alt", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyRightAlt, "Right Alt", nsInputSlotFlags::IsButton);

  RegisterInputSlot(nsInputSlot_KeyLeftWin, "Left Win", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyRightWin, "Right Win", nsInputSlotFlags::IsButton);

  RegisterInputSlot(nsInputSlot_KeyBracketOpen, "[", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyBracketClose, "]", nsInputSlotFlags::IsButton);

  RegisterInputSlot(nsInputSlot_KeySemicolon, ";", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyApostrophe, "'", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeySlash, "/", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyEquals, "=", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyTilde, "~", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyHyphen, "-", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyComma, ",", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyPeriod, ".", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyBackslash, "\\", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyPipe, "|", nsInputSlotFlags::IsButton);

  RegisterInputSlot(nsInputSlot_Key1, "1", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_Key2, "2", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_Key3, "3", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_Key4, "4", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_Key5, "5", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_Key6, "6", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_Key7, "7", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_Key8, "8", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_Key9, "9", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_Key0, "0", nsInputSlotFlags::IsButton);

  RegisterInputSlot(nsInputSlot_KeyNumpad1, "Numpad 1", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyNumpad2, "Numpad 2", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyNumpad3, "Numpad 3", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyNumpad4, "Numpad 4", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyNumpad5, "Numpad 5", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyNumpad6, "Numpad 6", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyNumpad7, "Numpad 7", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyNumpad8, "Numpad 8", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyNumpad9, "Numpad 9", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyNumpad0, "Numpad 0", nsInputSlotFlags::IsButton);

  RegisterInputSlot(nsInputSlot_KeyA, "A", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyB, "B", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyC, "C", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyD, "D", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyE, "E", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyF, "F", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyG, "G", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyH, "H", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyI, "I", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyJ, "J", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyK, "K", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyL, "L", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyM, "M", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyN, "N", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyO, "O", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyP, "P", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyQ, "Q", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyR, "R", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyS, "S", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyT, "T", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyU, "U", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyV, "V", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyW, "W", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyX, "X", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyY, "Y", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyZ, "Z", nsInputSlotFlags::IsButton);

  RegisterInputSlot(nsInputSlot_KeyF1, "F1", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyF2, "F2", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyF3, "F3", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyF4, "F4", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyF5, "F5", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyF6, "F6", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyF7, "F7", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyF8, "F8", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyF9, "F9", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyF10, "F10", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyF11, "F11", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyF12, "F12", nsInputSlotFlags::IsButton);

  RegisterInputSlot(nsInputSlot_KeyHome, "Home", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyEnd, "End", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyDelete, "Delete", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyInsert, "Insert", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyPageUp, "Page Up", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyPageDown, "Page Down", nsInputSlotFlags::IsButton);

  RegisterInputSlot(nsInputSlot_KeyNumLock, "Numlock", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyNumpadPlus, "Numpad +", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyNumpadMinus, "Numpad -", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyNumpadStar, "Numpad *", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyNumpadSlash, "Numpad /", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyNumpadPeriod, "Numpad .", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyNumpadEnter, "Enter", nsInputSlotFlags::IsButton);

  RegisterInputSlot(nsInputSlot_KeyCapsLock, "Capslock", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyPrint, "Print", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyScroll, "Scroll", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyPause, "Pause", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyApps, "Application", nsInputSlotFlags::IsButton);

  RegisterInputSlot(nsInputSlot_KeyPrevTrack, "Previous Track", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyNextTrack, "Next Track", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyPlayPause, "Play / Pause", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyStop, "Stop", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyVolumeUp, "Volume Up", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyVolumeDown, "Volume Down", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyMute, "Mute", nsInputSlotFlags::IsButton);

  RegisterInputSlot(nsInputSlot_MouseWheelUp, "Mousewheel Up", nsInputSlotFlags::IsMouseWheel);
  RegisterInputSlot(nsInputSlot_MouseWheelDown, "Mousewheel Down", nsInputSlotFlags::IsMouseWheel);

  RegisterInputSlot(nsInputSlot_MouseMoveNegX, "Mouse Move Left", nsInputSlotFlags::IsMouseAxisMove);
  RegisterInputSlot(nsInputSlot_MouseMovePosX, "Mouse Move Right", nsInputSlotFlags::IsMouseAxisMove);
  RegisterInputSlot(nsInputSlot_MouseMoveNegY, "Mouse Move Down", nsInputSlotFlags::IsMouseAxisMove);
  RegisterInputSlot(nsInputSlot_MouseMovePosY, "Mouse Move Up", nsInputSlotFlags::IsMouseAxisMove);

  RegisterInputSlot(nsInputSlot_MouseButton0, "Mousebutton 0", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_MouseButton1, "Mousebutton 1", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_MouseButton2, "Mousebutton 2", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_MouseButton3, "Mousebutton 3", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_MouseButton4, "Mousebutton 4", nsInputSlotFlags::IsButton);

  RegisterInputSlot(nsInputSlot_MouseDblClick0, "Left Double Click", nsInputSlotFlags::IsDoubleClick);
  RegisterInputSlot(nsInputSlot_MouseDblClick1, "Right Double Click", nsInputSlotFlags::IsDoubleClick);
  RegisterInputSlot(nsInputSlot_MouseDblClick2, "Middle Double Click", nsInputSlotFlags::IsDoubleClick);

  RegisterInputSlot(nsInputSlot_MousePositionX, "Mouse Position X", nsInputSlotFlags::IsMouseAxisPosition);
  RegisterInputSlot(nsInputSlot_MousePositionY, "Mouse Position Y", nsInputSlotFlags::IsMouseAxisPosition);


  RegisterInputSlot(nsInputSlot_TouchPoint0, "Touchpoint 0", nsInputSlotFlags::IsTouchPoint);
  RegisterInputSlot(nsInputSlot_TouchPoint0_PositionX, "Touchpoint 0 Position X", nsInputSlotFlags::IsTouchPosition);
  RegisterInputSlot(nsInputSlot_TouchPoint0_PositionY, "Touchpoint 0 Position Y", nsInputSlotFlags::IsTouchPosition);

  RegisterInputSlot(nsInputSlot_TouchPoint1, "Touchpoint 1", nsInputSlotFlags::IsTouchPoint);
  RegisterInputSlot(nsInputSlot_TouchPoint1_PositionX, "Touchpoint 1 Position X", nsInputSlotFlags::IsTouchPosition);
  RegisterInputSlot(nsInputSlot_TouchPoint1_PositionY, "Touchpoint 1 Position Y", nsInputSlotFlags::IsTouchPosition);

  RegisterInputSlot(nsInputSlot_TouchPoint2, "Touchpoint 2", nsInputSlotFlags::IsTouchPoint);
  RegisterInputSlot(nsInputSlot_TouchPoint2_PositionX, "Touchpoint 2 Position X", nsInputSlotFlags::IsTouchPosition);
  RegisterInputSlot(nsInputSlot_TouchPoint2_PositionY, "Touchpoint 2 Position Y", nsInputSlotFlags::IsTouchPosition);

  RegisterInputSlot(nsInputSlot_TouchPoint3, "Touchpoint 3", nsInputSlotFlags::IsTouchPoint);
  RegisterInputSlot(nsInputSlot_TouchPoint3_PositionX, "Touchpoint 3 Position X", nsInputSlotFlags::IsTouchPosition);
  RegisterInputSlot(nsInputSlot_TouchPoint3_PositionY, "Touchpoint 3 Position Y", nsInputSlotFlags::IsTouchPosition);

  RegisterInputSlot(nsInputSlot_TouchPoint4, "Touchpoint 4", nsInputSlotFlags::IsTouchPoint);
  RegisterInputSlot(nsInputSlot_TouchPoint4_PositionX, "Touchpoint 4 Position X", nsInputSlotFlags::IsTouchPosition);
  RegisterInputSlot(nsInputSlot_TouchPoint4_PositionY, "Touchpoint 4 Position Y", nsInputSlotFlags::IsTouchPosition);

  RegisterInputSlot(nsInputSlot_TouchPoint5, "Touchpoint 5", nsInputSlotFlags::IsTouchPoint);
  RegisterInputSlot(nsInputSlot_TouchPoint5_PositionX, "Touchpoint 5 Position X", nsInputSlotFlags::IsTouchPosition);
  RegisterInputSlot(nsInputSlot_TouchPoint5_PositionY, "Touchpoint 5 Position Y", nsInputSlotFlags::IsTouchPosition);

  RegisterInputSlot(nsInputSlot_TouchPoint6, "Touchpoint 6", nsInputSlotFlags::IsTouchPoint);
  RegisterInputSlot(nsInputSlot_TouchPoint6_PositionX, "Touchpoint 6 Position X", nsInputSlotFlags::IsTouchPosition);
  RegisterInputSlot(nsInputSlot_TouchPoint6_PositionY, "Touchpoint 6 Position Y", nsInputSlotFlags::IsTouchPosition);

  RegisterInputSlot(nsInputSlot_TouchPoint7, "Touchpoint 7", nsInputSlotFlags::IsTouchPoint);
  RegisterInputSlot(nsInputSlot_TouchPoint7_PositionX, "Touchpoint 7 Position X", nsInputSlotFlags::IsTouchPosition);
  RegisterInputSlot(nsInputSlot_TouchPoint7_PositionY, "Touchpoint 7 Position Y", nsInputSlotFlags::IsTouchPosition);

  RegisterInputSlot(nsInputSlot_TouchPoint8, "Touchpoint 8", nsInputSlotFlags::IsTouchPoint);
  RegisterInputSlot(nsInputSlot_TouchPoint8_PositionX, "Touchpoint 8 Position X", nsInputSlotFlags::IsTouchPosition);
  RegisterInputSlot(nsInputSlot_TouchPoint8_PositionY, "Touchpoint 8 Position Y", nsInputSlotFlags::IsTouchPosition);

  RegisterInputSlot(nsInputSlot_TouchPoint9, "Touchpoint 9", nsInputSlotFlags::IsTouchPoint);
  RegisterInputSlot(nsInputSlot_TouchPoint9_PositionX, "Touchpoint 9 Position X", nsInputSlotFlags::IsTouchPosition);
  RegisterInputSlot(nsInputSlot_TouchPoint9_PositionY, "Touchpoint 9 Position Y", nsInputSlotFlags::IsTouchPosition);
}

void nsStandardInputDevice::ResetInputSlotValues()
{
  m_InputSlotValues[nsInputSlot_MouseWheelUp] = 0;
  m_InputSlotValues[nsInputSlot_MouseWheelDown] = 0;
  m_InputSlotValues[nsInputSlot_MouseMoveNegX] = 0;
  m_InputSlotValues[nsInputSlot_MouseMovePosX] = 0;
  m_InputSlotValues[nsInputSlot_MouseMoveNegY] = 0;
  m_InputSlotValues[nsInputSlot_MouseMovePosY] = 0;
  m_InputSlotValues[nsInputSlot_MouseDblClick0] = 0;
  m_InputSlotValues[nsInputSlot_MouseDblClick1] = 0;
  m_InputSlotValues[nsInputSlot_MouseDblClick2] = 0;
}

void nsStandardInputDevice::UpdateInputSlotValues()
{
  const char* slotDown[5] = {nsInputSlot_MouseButton0, nsInputSlot_MouseButton1, nsInputSlot_MouseButton2, nsInputSlot_MouseButton3, nsInputSlot_MouseButton4};

  // don't read uninitialized values
  if (!m_InputSlotValues.Contains(slotDown[4]))
  {
    for (int i = 0; i < 5; ++i)
    {
      m_InputSlotValues[slotDown[i]] = 0;
    }
  }

  for (int i = 0; i < 5; ++i)
  {
    if (m_InputSlotValues[slotDown[i]] > 0)
    {
      if (m_uiMouseButtonReceivedUp[i] > 0)
      {
        --m_uiMouseButtonReceivedUp[i];
        m_InputSlotValues[slotDown[i]] = 0;
      }
    }
    else
    {
      if (m_uiMouseButtonReceivedDown[i] > 0)
      {
        --m_uiMouseButtonReceivedDown[i];
        m_InputSlotValues[slotDown[i]] = 1.0f;
      }
      // This is a workaround for a win32 bug: Double clicking on a title bar maximizes a window but only fires a single mouse up event. If that happens, no further clicks would be recognized because the balance between up and down events is broken. So if the slot is not signaled and there is no down event but an up event instead, we just consume it.
      else if (m_uiMouseButtonReceivedUp[i] > 0)
      {
        --m_uiMouseButtonReceivedUp[i];
        m_InputSlotValues[slotDown[i]] = 0;
      }
    }
  }

  SUPER::UpdateInputSlotValues();
}

void nsStandardInputDevice::ApplyClipRect(nsMouseCursorClipMode::Enum mode, nsMinWindows::HWND hWnd)
{
  if (!m_bApplyClipRect)
    return;

  m_bApplyClipRect = false;

  if (mode == nsMouseCursorClipMode::NoClip)
  {
    ClipCursor(nullptr);
    return;
  }

  RECT r;
  {
    RECT area;
    GetClientRect(nsMinWindows::ToNative(hWnd), &area);
    POINT p0, p1;
    p0.x = 0;
    p0.y = 0;
    p1.x = area.right;
    p1.y = area.bottom;

    ClientToScreen(nsMinWindows::ToNative(hWnd), &p0);
    ClientToScreen(nsMinWindows::ToNative(hWnd), &p1);

    r.top = p0.y;
    r.left = p0.x;
    r.right = p1.x;
    r.bottom = p1.y;
  }

  if (mode == nsMouseCursorClipMode::ClipToPosition)
  {
    POINT mp;
    if (GetCursorPos(&mp))
    {
      // center the position inside the window rect
      mp.x = r.left + (r.right - r.left) / 2;
      mp.y = r.top + (r.bottom - r.top) / 2;

      r.top = mp.y;
      r.bottom = mp.y;
      r.left = mp.x;
      r.right = mp.x;
    }
  }

  ClipCursor(&r);
}

void nsStandardInputDevice::SetClipMouseCursor(nsMouseCursorClipMode::Enum mode)
{
  if (m_ClipCursorMode == mode)
    return;

  m_ClipCursorMode = mode;
  m_bApplyClipRect = m_ClipCursorMode != nsMouseCursorClipMode::NoClip;

  if (m_ClipCursorMode == nsMouseCursorClipMode::NoClip)
    ClipCursor(nullptr);
}

// WM_INPUT mouse clicks do not work in some VMs.
// When this is enabled, mouse clicks are retrieved via standard WM_LBUTTONDOWN.
#define NS_MOUSEBUTTON_COMPATIBILTY_MODE NS_ON

void nsStandardInputDevice::WindowMessage(
  nsMinWindows::HWND hWnd, nsMinWindows::UINT msg, nsMinWindows::WPARAM wparam, nsMinWindows::LPARAM lparam)
{
#if NS_ENABLED(NS_MOUSEBUTTON_COMPATIBILTY_MODE)
  static nsInt32 s_iMouseCaptureCount = 0;
#endif

  switch (msg)
  {
    case WM_MOUSEWHEEL:
    {
      // The mousewheel does not work with rawinput over touchpads (at least not all)
      // So we handle that one individually

      const nsInt32 iRotated = (nsInt16)HIWORD(wparam);

      if (iRotated > 0)
        m_InputSlotValues[nsInputSlot_MouseWheelUp] = iRotated / 120.0f;
      else
        m_InputSlotValues[nsInputSlot_MouseWheelDown] = iRotated / -120.0f;

      break;
    }

    case WM_MOUSEMOVE:
    {
      RECT area;
      GetClientRect(nsMinWindows::ToNative(hWnd), &area);

      const nsUInt32 uiResX = area.right - area.left;
      const nsUInt32 uiResY = area.bottom - area.top;

      const float fPosX = (float)((short)LOWORD(lparam));
      const float fPosY = (float)((short)HIWORD(lparam));

      s_iMouseIsOverWindowNumber = m_uiWindowNumber;
      m_InputSlotValues[nsInputSlot_MousePositionX] = (fPosX / uiResX);
      m_InputSlotValues[nsInputSlot_MousePositionY] = (fPosY / uiResY);

      if (m_ClipCursorMode == nsMouseCursorClipMode::ClipToPosition || m_ClipCursorMode == nsMouseCursorClipMode::ClipToWindowImmediate)
      {
        ApplyClipRect(m_ClipCursorMode, hWnd);
      }

      break;
    }

    case WM_SETFOCUS:
    {
      m_bApplyClipRect = true;
      ApplyClipRect(m_ClipCursorMode, hWnd);
      break;
    }

    case WM_KILLFOCUS:
    {
      OnFocusLost(hWnd);
      return;
    }

    case WM_CHAR:
      m_uiLastCharacter = (wchar_t)wparam;
      return;

      // these messages would only arrive, if the window had the flag CS_DBLCLKS
      // see https://docs.microsoft.com/windows/win32/inputdev/wm-lbuttondblclk
      // this would add lag and hide single clicks when the user double clicks
      // therefore it is not used
      // case WM_LBUTTONDBLCLK:
      //  m_InputSlotValues[nsInputSlot_MouseDblClick0] = 1.0f;
      //  return;
      // case WM_RBUTTONDBLCLK:
      //  m_InputSlotValues[nsInputSlot_MouseDblClick1] = 1.0f;
      //  return;
      // case WM_MBUTTONDBLCLK:
      //  m_InputSlotValues[nsInputSlot_MouseDblClick2] = 1.0f;
      //  return;

#if NS_ENABLED(NS_MOUSEBUTTON_COMPATIBILTY_MODE)

    case WM_LBUTTONDOWN:
      m_uiMouseButtonReceivedDown[0]++;

      if (s_iMouseCaptureCount == 0)
        SetCapture(nsMinWindows::ToNative(hWnd));
      ++s_iMouseCaptureCount;


      return;

    case WM_LBUTTONUP:
      m_uiMouseButtonReceivedUp[0]++;
      ApplyClipRect(m_ClipCursorMode, hWnd);

      --s_iMouseCaptureCount;
      if (s_iMouseCaptureCount <= 0)
        ReleaseCapture();

      return;

    case WM_RBUTTONDOWN:
      m_uiMouseButtonReceivedDown[1]++;

      if (s_iMouseCaptureCount == 0)
        SetCapture(nsMinWindows::ToNative(hWnd));
      ++s_iMouseCaptureCount;

      return;

    case WM_RBUTTONUP:
      m_uiMouseButtonReceivedUp[1]++;
      ApplyClipRect(m_ClipCursorMode, hWnd);

      --s_iMouseCaptureCount;
      if (s_iMouseCaptureCount <= 0)
        ReleaseCapture();


      return;

    case WM_MBUTTONDOWN:
      m_uiMouseButtonReceivedDown[2]++;

      if (s_iMouseCaptureCount == 0)
        SetCapture(nsMinWindows::ToNative(hWnd));
      ++s_iMouseCaptureCount;
      return;

    case WM_MBUTTONUP:
      m_uiMouseButtonReceivedUp[2]++;

      --s_iMouseCaptureCount;
      if (s_iMouseCaptureCount <= 0)
        ReleaseCapture();

      return;

    case WM_XBUTTONDOWN:
      if (GET_XBUTTON_WPARAM(wparam) == XBUTTON1)
        m_uiMouseButtonReceivedDown[3]++;
      if (GET_XBUTTON_WPARAM(wparam) == XBUTTON2)
        m_uiMouseButtonReceivedDown[4]++;

      if (s_iMouseCaptureCount == 0)
        SetCapture(nsMinWindows::ToNative(hWnd));
      ++s_iMouseCaptureCount;

      return;

    case WM_XBUTTONUP:
      if (GET_XBUTTON_WPARAM(wparam) == XBUTTON1)
        m_uiMouseButtonReceivedUp[3]++;
      if (GET_XBUTTON_WPARAM(wparam) == XBUTTON2)
        m_uiMouseButtonReceivedUp[4]++;

      --s_iMouseCaptureCount;
      if (s_iMouseCaptureCount <= 0)
        ReleaseCapture();

      return;

    case WM_CAPTURECHANGED: // Sent to the window that is losing the mouse capture.
      s_iMouseCaptureCount = 0;
      return;

#else

    case WM_LBUTTONUP:
      ApplyClipRect(m_bClipCursor, hWnd);
      return;

#endif

    case WM_INPUT:
    {
      nsUInt32 uiSize = 0;

      GetRawInputData((HRAWINPUT)lparam, RID_INPUT, nullptr, &uiSize, sizeof(RAWINPUTHEADER));

      if (uiSize == 0)
        return;

      nsHybridArray<nsUInt8, sizeof(RAWINPUT)> InputData;
      InputData.SetCountUninitialized(uiSize);

      if (GetRawInputData((HRAWINPUT)lparam, RID_INPUT, &InputData[0], &uiSize, sizeof(RAWINPUTHEADER)) != uiSize)
        return;

      RAWINPUT* raw = (RAWINPUT*)&InputData[0];

      if (raw->header.dwType == RIM_TYPEKEYBOARD)
      {
        static bool bIgnoreNext = false;

        if (bIgnoreNext)
        {
          bIgnoreNext = false;
          return;
        }

        static bool bWasStupidLeftShift = false;

        const nsUInt8 uiScanCode = static_cast<nsUInt8>(raw->data.keyboard.MakeCode);
        const bool bIsExtended = (raw->data.keyboard.Flags & RI_KEY_E0) != 0;

        if (uiScanCode == 42 && bIsExtended) // 42 has to be special I guess
        {
          bWasStupidLeftShift = true;
          return;
        }

        nsStringView sInputSlotName = nsInputManager::ConvertScanCodeToEngineName(uiScanCode, bIsExtended);

        // On Windows this only happens with the Pause key, but it will actually send the 'Right Ctrl' key value
        // so we need to fix this manually
        if (raw->data.keyboard.Flags & RI_KEY_E1)
        {
          sInputSlotName = nsInputSlot_KeyPause;
          bIgnoreNext = true;
        }

        // The Print key is sent as a two key sequence, first an 'extended left shift' and then the Numpad* key is sent
        // we ignore the first stupid shift key entirely and then modify the following Numpad* key
        // Note that the 'stupid shift' is sent along with several other keys as well (e.g. left/right/up/down arrows)
        // in these cases we can ignore them entirely, as the following key will have an unambiguous key code
        if (sInputSlotName == nsInputSlot_KeyNumpadStar && bWasStupidLeftShift)
          sInputSlotName = nsInputSlot_KeyPrint;

        bWasStupidLeftShift = false;

        const bool bPressed = !(raw->data.keyboard.Flags & 0x01);

        m_InputSlotValues[sInputSlotName] = bPressed ? 1.0f : 0.0f;

        if ((m_InputSlotValues[nsInputSlot_KeyLeftCtrl] > 0.1f) && (m_InputSlotValues[nsInputSlot_KeyLeftAlt] > 0.1f) &&
            (m_InputSlotValues[nsInputSlot_KeyNumpadEnter] > 0.1f))
        {
          switch (GetClipMouseCursor())
          {
            case nsMouseCursorClipMode::NoClip:
              SetClipMouseCursor(nsMouseCursorClipMode::ClipToWindow);
              break;

            default:
              SetClipMouseCursor(nsMouseCursorClipMode::NoClip);
              break;
          }
        }
      }
      else if (raw->header.dwType == RIM_TYPEMOUSE)
      {
        const nsUInt32 uiButtons = raw->data.mouse.usButtonFlags;

        // "absolute" positions are only reported by devices such as Pens
        // if at all, we should handle them as touch points, not as mouse positions
        if ((raw->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE) == 0)
        {
          m_InputSlotValues[nsInputSlot_MouseMoveNegX] +=
            ((raw->data.mouse.lLastX < 0) ? (float)-raw->data.mouse.lLastX : 0.0f) * GetMouseSpeed().x;
          m_InputSlotValues[nsInputSlot_MouseMovePosX] +=
            ((raw->data.mouse.lLastX > 0) ? (float)raw->data.mouse.lLastX : 0.0f) * GetMouseSpeed().x;
          m_InputSlotValues[nsInputSlot_MouseMoveNegY] +=
            ((raw->data.mouse.lLastY < 0) ? (float)-raw->data.mouse.lLastY : 0.0f) * GetMouseSpeed().y;
          m_InputSlotValues[nsInputSlot_MouseMovePosY] +=
            ((raw->data.mouse.lLastY > 0) ? (float)raw->data.mouse.lLastY : 0.0f) * GetMouseSpeed().y;

// Mouse input does not always work via WM_INPUT
// e.g. some VMs don't send mouse click input via WM_INPUT when the mouse cursor is visible
// therefore in 'compatibility mode' it is just queried via standard WM_LBUTTONDOWN etc.
// to get 'high performance' mouse clicks, this code would work fine though
// but I doubt it makes much difference in latency
#if NS_DISABLED(NS_MOUSEBUTTON_COMPATIBILTY_MODE)
          for (nsInt32 mb = 0; mb < 5; ++mb)
          {
            char szTemp[32];
            nsStringUtils::snprintf(szTemp, 32, "mouse_button_%i", mb);

            if ((uiButtons & (RI_MOUSE_BUTTON_1_DOWN << (mb * 2))) != 0)
              m_InputSlotValues[szTemp] = 1.0f;

            if ((uiButtons & (RI_MOUSE_BUTTON_1_DOWN << (mb * 2 + 1))) != 0)
              m_InputSlotValues[szTemp] = 0.0f;
          }
#endif
        }
        else if ((raw->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE) != 0)
        {
          if ((raw->data.mouse.usFlags & MOUSE_VIRTUAL_DESKTOP) != 0)
          {
            // if this flag is set, we are getting mouse input through a remote desktop session
            // and that means we will not get any relative mouse move events, so we need to emulate them

            static const nsInt32 iVirtualDesktopW = GetSystemMetrics(SM_CXVIRTUALSCREEN);
            static const nsInt32 iVirtualDesktopH = GetSystemMetrics(SM_CYVIRTUALSCREEN);

            static nsVec2 vLastPos(nsMath::MaxValue<float>());
            const nsVec2 vNewPos(
              (raw->data.mouse.lLastX / 65535.0f) * iVirtualDesktopW, (raw->data.mouse.lLastY / 65535.0f) * iVirtualDesktopH);

            if (vLastPos.x != nsMath::MaxValue<float>())
            {
              const nsVec2 vDiff = vNewPos - vLastPos;

              m_InputSlotValues[nsInputSlot_MouseMoveNegX] += ((vDiff.x < 0) ? (float)-vDiff.x : 0.0f) * GetMouseSpeed().x;
              m_InputSlotValues[nsInputSlot_MouseMovePosX] += ((vDiff.x > 0) ? (float)vDiff.x : 0.0f) * GetMouseSpeed().x;
              m_InputSlotValues[nsInputSlot_MouseMoveNegY] += ((vDiff.y < 0) ? (float)-vDiff.y : 0.0f) * GetMouseSpeed().y;
              m_InputSlotValues[nsInputSlot_MouseMovePosY] += ((vDiff.y > 0) ? (float)vDiff.y : 0.0f) * GetMouseSpeed().y;
            }

            vLastPos = vNewPos;
          }
          else
          {
            static int iTouchPoint = 0;

            nsStringView sSlot = nsInputManager::GetInputSlotTouchPoint(iTouchPoint);
            nsStringView sSlotX = nsInputManager::GetInputSlotTouchPointPositionX(iTouchPoint);
            nsStringView sSlotY = nsInputManager::GetInputSlotTouchPointPositionY(iTouchPoint);

            m_InputSlotValues[sSlotX] = (raw->data.mouse.lLastX / 65535.0f) + m_uiWindowNumber;
            m_InputSlotValues[sSlotY] = (raw->data.mouse.lLastY / 65535.0f);

            if ((uiButtons & (RI_MOUSE_BUTTON_1_DOWN | RI_MOUSE_BUTTON_2_DOWN)) != 0)
            {
              m_InputSlotValues[sSlot] = 1.0f;
            }

            if ((uiButtons & (RI_MOUSE_BUTTON_1_UP | RI_MOUSE_BUTTON_2_UP)) != 0)
            {
              m_InputSlotValues[sSlot] = 0.0f;
            }
          }
        }
        else
        {
          nsLog::Info("Unknown Mouse Move: {0} | {1}, Flags = {2}", nsArgF(raw->data.mouse.lLastX, 1), nsArgF(raw->data.mouse.lLastY, 1),
            (nsUInt32)raw->data.mouse.usFlags);
        }
      }
    }
  }
}


static void SetKeyNameForScanCode(int iScanCode, bool bExtended, const char* szInputSlot)
{
  const nsUInt32 uiKeyCode = (iScanCode << 16) | (bExtended ? (1 << 24) : 0);

  wchar_t szKeyName[32] = {0};
  GetKeyNameTextW(uiKeyCode, szKeyName, 30);

  nsStringUtf8 sName(szKeyName);

  nsLog::Dev("Translated '{0}' to '{1}'", nsInputManager::GetInputSlotDisplayName(szInputSlot), sName.GetData());

  nsInputManager::SetInputSlotDisplayName(szInputSlot, sName.GetData());
}

void nsStandardInputDevice::LocalizeButtonDisplayNames()
{
  NS_LOG_BLOCK("nsStandardInputDevice::LocalizeButtonDisplayNames");

  SetKeyNameForScanCode(1, false, nsInputSlot_KeyEscape);
  SetKeyNameForScanCode(2, false, nsInputSlot_Key1);
  SetKeyNameForScanCode(3, false, nsInputSlot_Key2);
  SetKeyNameForScanCode(4, false, nsInputSlot_Key3);
  SetKeyNameForScanCode(5, false, nsInputSlot_Key4);
  SetKeyNameForScanCode(6, false, nsInputSlot_Key5);
  SetKeyNameForScanCode(7, false, nsInputSlot_Key6);
  SetKeyNameForScanCode(8, false, nsInputSlot_Key7);
  SetKeyNameForScanCode(9, false, nsInputSlot_Key8);
  SetKeyNameForScanCode(10, false, nsInputSlot_Key9);
  SetKeyNameForScanCode(11, false, nsInputSlot_Key0);

  SetKeyNameForScanCode(12, false, nsInputSlot_KeyHyphen);
  SetKeyNameForScanCode(13, false, nsInputSlot_KeyEquals);
  SetKeyNameForScanCode(14, false, nsInputSlot_KeyBackspace);

  SetKeyNameForScanCode(15, false, nsInputSlot_KeyTab);
  SetKeyNameForScanCode(16, false, nsInputSlot_KeyQ);
  SetKeyNameForScanCode(17, false, nsInputSlot_KeyW);
  SetKeyNameForScanCode(18, false, nsInputSlot_KeyE);
  SetKeyNameForScanCode(19, false, nsInputSlot_KeyR);
  SetKeyNameForScanCode(20, false, nsInputSlot_KeyT);
  SetKeyNameForScanCode(21, false, nsInputSlot_KeyY);
  SetKeyNameForScanCode(22, false, nsInputSlot_KeyU);
  SetKeyNameForScanCode(23, false, nsInputSlot_KeyI);
  SetKeyNameForScanCode(24, false, nsInputSlot_KeyO);
  SetKeyNameForScanCode(25, false, nsInputSlot_KeyP);
  SetKeyNameForScanCode(26, false, nsInputSlot_KeyBracketOpen);
  SetKeyNameForScanCode(27, false, nsInputSlot_KeyBracketClose);
  SetKeyNameForScanCode(28, false, nsInputSlot_KeyReturn);

  SetKeyNameForScanCode(29, false, nsInputSlot_KeyLeftCtrl);
  SetKeyNameForScanCode(30, false, nsInputSlot_KeyA);
  SetKeyNameForScanCode(31, false, nsInputSlot_KeyS);
  SetKeyNameForScanCode(32, false, nsInputSlot_KeyD);
  SetKeyNameForScanCode(33, false, nsInputSlot_KeyF);
  SetKeyNameForScanCode(34, false, nsInputSlot_KeyG);
  SetKeyNameForScanCode(35, false, nsInputSlot_KeyH);
  SetKeyNameForScanCode(36, false, nsInputSlot_KeyJ);
  SetKeyNameForScanCode(37, false, nsInputSlot_KeyK);
  SetKeyNameForScanCode(38, false, nsInputSlot_KeyL);
  SetKeyNameForScanCode(39, false, nsInputSlot_KeySemicolon);
  SetKeyNameForScanCode(40, false, nsInputSlot_KeyApostrophe);

  SetKeyNameForScanCode(41, false, nsInputSlot_KeyTilde);
  SetKeyNameForScanCode(42, false, nsInputSlot_KeyLeftShift);
  SetKeyNameForScanCode(43, false, nsInputSlot_KeyBackslash);

  SetKeyNameForScanCode(44, false, nsInputSlot_KeyZ);
  SetKeyNameForScanCode(45, false, nsInputSlot_KeyX);
  SetKeyNameForScanCode(46, false, nsInputSlot_KeyC);
  SetKeyNameForScanCode(47, false, nsInputSlot_KeyV);
  SetKeyNameForScanCode(48, false, nsInputSlot_KeyB);
  SetKeyNameForScanCode(49, false, nsInputSlot_KeyN);
  SetKeyNameForScanCode(50, false, nsInputSlot_KeyM);
  SetKeyNameForScanCode(51, false, nsInputSlot_KeyComma);
  SetKeyNameForScanCode(52, false, nsInputSlot_KeyPeriod);
  SetKeyNameForScanCode(53, false, nsInputSlot_KeySlash);
  SetKeyNameForScanCode(54, false, nsInputSlot_KeyRightShift);

  SetKeyNameForScanCode(55, false, nsInputSlot_KeyNumpadStar); // Overlaps with Print

  SetKeyNameForScanCode(56, false, nsInputSlot_KeyLeftAlt);
  SetKeyNameForScanCode(57, false, nsInputSlot_KeySpace);
  SetKeyNameForScanCode(58, false, nsInputSlot_KeyCapsLock);

  SetKeyNameForScanCode(59, false, nsInputSlot_KeyF1);
  SetKeyNameForScanCode(60, false, nsInputSlot_KeyF2);
  SetKeyNameForScanCode(61, false, nsInputSlot_KeyF3);
  SetKeyNameForScanCode(62, false, nsInputSlot_KeyF4);
  SetKeyNameForScanCode(63, false, nsInputSlot_KeyF5);
  SetKeyNameForScanCode(64, false, nsInputSlot_KeyF6);
  SetKeyNameForScanCode(65, false, nsInputSlot_KeyF7);
  SetKeyNameForScanCode(66, false, nsInputSlot_KeyF8);
  SetKeyNameForScanCode(67, false, nsInputSlot_KeyF9);
  SetKeyNameForScanCode(68, false, nsInputSlot_KeyF10);

  SetKeyNameForScanCode(69, true, nsInputSlot_KeyNumLock);       // Prints 'Pause' if it is not 'extended'
  SetKeyNameForScanCode(70, false, nsInputSlot_KeyScroll);       // This overlaps with Pause

  SetKeyNameForScanCode(71, false, nsInputSlot_KeyNumpad7);      // This overlaps with Home
  SetKeyNameForScanCode(72, false, nsInputSlot_KeyNumpad8);      // This overlaps with Arrow Up
  SetKeyNameForScanCode(73, false, nsInputSlot_KeyNumpad9);      // This overlaps with Page Up
  SetKeyNameForScanCode(74, false, nsInputSlot_KeyNumpadMinus);

  SetKeyNameForScanCode(75, false, nsInputSlot_KeyNumpad4);      // This overlaps with Arrow Left
  SetKeyNameForScanCode(76, false, nsInputSlot_KeyNumpad5);
  SetKeyNameForScanCode(77, false, nsInputSlot_KeyNumpad6);      // This overlaps with Arrow Right
  SetKeyNameForScanCode(78, false, nsInputSlot_KeyNumpadPlus);

  SetKeyNameForScanCode(79, false, nsInputSlot_KeyNumpad1);      // This overlaps with End
  SetKeyNameForScanCode(80, false, nsInputSlot_KeyNumpad2);      // This overlaps with Arrow Down
  SetKeyNameForScanCode(81, false, nsInputSlot_KeyNumpad3);      // This overlaps with Page Down
  SetKeyNameForScanCode(82, false, nsInputSlot_KeyNumpad0);      // This overlaps with Insert
  SetKeyNameForScanCode(83, false, nsInputSlot_KeyNumpadPeriod); // This overlaps with Insert

  SetKeyNameForScanCode(86, false, nsInputSlot_KeyPipe);

  SetKeyNameForScanCode(87, false, "keyboard_f11");
  SetKeyNameForScanCode(88, false, "keyboard_f12");

  SetKeyNameForScanCode(91, true, nsInputSlot_KeyLeftWin);  // Prints '' if it is not 'extended'
  SetKeyNameForScanCode(92, true, nsInputSlot_KeyRightWin); // Prints '' if it is not 'extended'
  SetKeyNameForScanCode(93, true, nsInputSlot_KeyApps);     // Prints '' if it is not 'extended'

  // 'Extended' keys
  SetKeyNameForScanCode(28, true, nsInputSlot_KeyNumpadEnter);
  SetKeyNameForScanCode(29, true, nsInputSlot_KeyRightCtrl);
  SetKeyNameForScanCode(53, true, nsInputSlot_KeyNumpadSlash);
  SetKeyNameForScanCode(55, true, nsInputSlot_KeyPrint);
  SetKeyNameForScanCode(56, true, nsInputSlot_KeyRightAlt);
  SetKeyNameForScanCode(70, true, nsInputSlot_KeyPause);
  SetKeyNameForScanCode(71, true, nsInputSlot_KeyHome);
  SetKeyNameForScanCode(72, true, nsInputSlot_KeyUp);
  SetKeyNameForScanCode(73, true, nsInputSlot_KeyPageUp);

  SetKeyNameForScanCode(75, true, nsInputSlot_KeyLeft);
  SetKeyNameForScanCode(77, true, nsInputSlot_KeyRight);

  SetKeyNameForScanCode(79, true, nsInputSlot_KeyEnd);
  SetKeyNameForScanCode(80, true, nsInputSlot_KeyDown);
  SetKeyNameForScanCode(81, true, nsInputSlot_KeyPageDown);
  SetKeyNameForScanCode(82, true, nsInputSlot_KeyInsert);
  SetKeyNameForScanCode(83, true, nsInputSlot_KeyDelete);
}

void nsStandardInputDevice::SetShowMouseCursor(bool bShow)
{
  if (m_bShowCursor == bShow)
    return;

  m_bShowCursor = bShow;
  ShowCursor(m_bShowCursor);
}

bool nsStandardInputDevice::GetShowMouseCursor() const
{
  return m_bShowCursor;
}

void nsStandardInputDevice::OnFocusLost(nsMinWindows::HWND hWnd)
{
  m_bApplyClipRect = true;
  ApplyClipRect(nsMouseCursorClipMode::NoClip, hWnd);

  auto it = m_InputSlotValues.GetIterator();

  while (it.IsValid())
  {
    it.Value() = 0.0f;
    it.Next();
  }


  const char* slotDown[5] = {nsInputSlot_MouseButton0, nsInputSlot_MouseButton1, nsInputSlot_MouseButton2, nsInputSlot_MouseButton3, nsInputSlot_MouseButton4};

  static_assert(NS_ARRAY_SIZE(m_uiMouseButtonReceivedDown) == NS_ARRAY_SIZE(slotDown));

  for (int i = 0; i < NS_ARRAY_SIZE(m_uiMouseButtonReceivedDown); ++i)
  {
    m_uiMouseButtonReceivedDown[i] = 0;
    m_uiMouseButtonReceivedUp[i] = 0;

    m_InputSlotValues[slotDown[i]] = 0;
  }
}
