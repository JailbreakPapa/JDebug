#include <Core/Input/InputManager.h>
#include <Core/System/Implementation/uwp/InputDevice_uwp.h>
#include <Foundation/Basics/Platform/uwp/UWPUtils.h>
#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Strings/StringConversion.h>
#include <wrl/event.h>

using namespace ABI::Windows::UI::Core;

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsStandardInputDevice, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsStandardInputDevice::nsStandardInputDevice(ICoreWindow* coreWindow)
  : m_coreWindow(coreWindow)
{
  // TODO
  m_ClipCursorMode = nsMouseCursorClipMode::NoClip;
  m_bShowCursor = true;
}

nsStandardInputDevice::~nsStandardInputDevice()
{
  if (m_coreWindow)
  {
    m_coreWindow->remove_KeyDown(m_eventRegistration_keyDown);
    m_coreWindow->remove_KeyUp(m_eventRegistration_keyUp);
    m_coreWindow->remove_CharacterReceived(m_eventRegistration_characterReceived);
    m_coreWindow->remove_PointerMoved(m_eventRegistration_pointerMoved);
    m_coreWindow->remove_PointerEntered(m_eventRegistration_pointerEntered);
    m_coreWindow->remove_PointerExited(m_eventRegistration_pointerExited);
    m_coreWindow->remove_PointerCaptureLost(m_eventRegistration_pointerCaptureLost);
    m_coreWindow->remove_PointerPressed(m_eventRegistration_pointerPressed);
    m_coreWindow->remove_PointerReleased(m_eventRegistration_pointerReleased);
    m_coreWindow->remove_PointerWheelChanged(m_eventRegistration_pointerWheelChanged);
  }

  if (m_mouseDevice)
  {
    m_mouseDevice->remove_MouseMoved(m_eventRegistration_mouseMoved);
  }
}

void nsStandardInputDevice::InitializeDevice()
{
  using KeyHandler = __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CKeyEventArgs;
  using CharacterReceivedHandler =
    __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CCharacterReceivedEventArgs;
  using PointerHander = __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CPointerEventArgs;

  // Keyboard
  m_coreWindow->add_KeyDown(Callback<KeyHandler>(this, &nsStandardInputDevice::OnKeyEvent).Get(), &m_eventRegistration_keyDown);
  m_coreWindow->add_KeyUp(Callback<KeyHandler>(this, &nsStandardInputDevice::OnKeyEvent).Get(), &m_eventRegistration_keyUp);
  m_coreWindow->add_CharacterReceived(Callback<CharacterReceivedHandler>(this, &nsStandardInputDevice::OnCharacterReceived).Get(),
    &m_eventRegistration_characterReceived);

  // Pointer
  // Note that a pointer may be mouse, pen/stylus or touch!
  // We bundle move/press/enter all in a single callback to update all pointer state - all these cases have in common that pen/touch is
  // pressed now.
  m_coreWindow->add_PointerMoved(Callback<PointerHander>(this, &nsStandardInputDevice::OnPointerMovePressEnter).Get(),
    &m_eventRegistration_pointerMoved);
  m_coreWindow->add_PointerEntered(Callback<PointerHander>(this, &nsStandardInputDevice::OnPointerMovePressEnter).Get(),
    &m_eventRegistration_pointerEntered);
  m_coreWindow->add_PointerPressed(Callback<PointerHander>(this, &nsStandardInputDevice::OnPointerMovePressEnter).Get(),
    &m_eventRegistration_pointerPressed);
  // Changes in the pointer wheel:
  m_coreWindow->add_PointerWheelChanged(Callback<PointerHander>(this, &nsStandardInputDevice::OnPointerWheelChange).Get(),
    &m_eventRegistration_pointerWheelChanged);
  // Exit for touch or stylus means that we no longer have a press.
  // However, we presserve mouse button presses.
  m_coreWindow->add_PointerExited(Callback<PointerHander>(this, &nsStandardInputDevice::OnPointerReleasedOrExited).Get(),
    &m_eventRegistration_pointerExited);
  m_coreWindow->add_PointerReleased(Callback<PointerHander>(this, &nsStandardInputDevice::OnPointerReleasedOrExited).Get(),
    &m_eventRegistration_pointerReleased);
  // Capture loss.
  // From documentation "Occurs when a pointer moves to another app. This event is raised after PointerExited and is the final event
  // received by the app for this pointer." If this happens we want to release all mouse buttons as well.
  m_coreWindow->add_PointerCaptureLost(Callback<PointerHander>(this, &nsStandardInputDevice::OnPointerCaptureLost).Get(),
    &m_eventRegistration_pointerCaptureLost);


  // Mouse
  // The only thing that we get from the MouseDevice class is mouse moved which gives us unfiltered relative mouse position.
  // Everything else is done by WinRt's "Pointer"
  // https://docs.microsoft.com/uwp/api/windows.devices.input.mousedevice
  // Relevant article for mouse move:
  // https://docs.microsoft.com/windows/uwp/gaming/relative-mouse-movement
  {
    ComPtr<ABI::Windows::Devices::Input::IMouseDeviceStatics> mouseDeviceStatics;
    if (SUCCEEDED(ABI::Windows::Foundation::GetActivationFactory(HStringReference(RuntimeClass_Windows_Devices_Input_MouseDevice).Get(),
          &mouseDeviceStatics)))
    {
      if (SUCCEEDED(mouseDeviceStatics->GetForCurrentView(&m_mouseDevice)))
      {
        using MouseMovedHandler =
          __FITypedEventHandler_2_Windows__CDevices__CInput__CMouseDevice_Windows__CDevices__CInput__CMouseEventArgs;
        m_mouseDevice->add_MouseMoved(Callback<MouseMovedHandler>(this, &nsStandardInputDevice::OnMouseMoved).Get(),
          &m_eventRegistration_mouseMoved);
      }
    }
  }
}

HRESULT nsStandardInputDevice::OnKeyEvent(ICoreWindow* coreWindow, IKeyEventArgs* args)
{
  // Closely related to the RawInput implementation in Win32/InputDevice_win32.inl

  CorePhysicalKeyStatus keyStatus;
  NS_SUCCEED_OR_RETURN(args->get_KeyStatus(&keyStatus));

  static bool bWasStupidLeftShift = false;

  if (keyStatus.ScanCode == 42 && keyStatus.IsExtendedKey) // 42 has to be special I guess
  {
    bWasStupidLeftShift = true;
    return S_OK;
  }

  nsStringView sInputSlotName = nsInputManager::ConvertScanCodeToEngineName(static_cast<nsUInt8>(keyStatus.ScanCode), keyStatus.IsExtendedKey == TRUE);
  if (sInputSlotName.IsEmpty())
    return S_OK;


  // Don't know yet how to handle this in UWP:

  // On Windows this only happens with the Pause key, but it will actually send the 'Right Ctrl' key value
  // so we need to fix this manually
  // if (raw->data.keyboard.Flags & RI_KEY_E1)
  //{
  //  szInputSlotName = nsInputSlot_KeyPause;
  //  bIgnoreNext = true;
  //}


  // The Print key is sent as a two key sequence, first an 'extended left shift' and then the Numpad* key is sent
  // we ignore the first stupid shift key entirely and then modify the following Numpad* key
  // Note that the 'stupid shift' is sent along with several other keys as well (e.g. left/right/up/down arrows)
  // in these cases we can ignore them entirely, as the following key will have an unambiguous key code
  if (sInputSlotName == nsInputSlot_KeyNumpadStar && bWasStupidLeftShift)
    sInputSlotName = nsInputSlot_KeyPrint;

  bWasStupidLeftShift = false;

  m_InputSlotValues[sInputSlotName] = keyStatus.IsKeyReleased ? 0.0f : 1.0f;

  return S_OK;
}

HRESULT nsStandardInputDevice::OnCharacterReceived(ICoreWindow* coreWindow, ICharacterReceivedEventArgs* args)
{
  UINT32 keyCode = 0;
  NS_SUCCEED_OR_RETURN(args->get_KeyCode(&keyCode));
  m_uiLastCharacter = keyCode;

  return S_OK;
}

HRESULT nsStandardInputDevice::OnPointerMovePressEnter(ICoreWindow* coreWindow, IPointerEventArgs* args)
{
  using namespace ABI::Windows::Devices::Input;

  ComPtr<ABI::Windows::UI::Input::IPointerPoint> pointerPoint;
  NS_SUCCEED_OR_RETURN(args->get_CurrentPoint(&pointerPoint));
  ComPtr<IPointerDevice> pointerDevice;
  NS_SUCCEED_OR_RETURN(pointerPoint->get_PointerDevice(&pointerDevice));
  PointerDeviceType deviceType;
  NS_SUCCEED_OR_RETURN(pointerDevice->get_PointerDeviceType(&deviceType));

  // Pointer position.
  // From the documentation: "The position of the pointer in device-independent pixel (DIP)."
  // Note also, that there is "raw position" which may be free of pointer prediction etc.
  ABI::Windows::Foundation::Point pointerPosition;
  NS_SUCCEED_OR_RETURN(pointerPoint->get_Position(&pointerPosition));
  ABI::Windows::Foundation::Rect windowRectangle;
  NS_SUCCEED_OR_RETURN(coreWindow->get_Bounds(&windowRectangle)); // Bounds are in DIP as well!

  float relativePosX = static_cast<float>(pointerPosition.X) / windowRectangle.Width;
  float relativePosY = static_cast<float>(pointerPosition.Y) / windowRectangle.Height;

  if (deviceType == PointerDeviceType_Mouse)
  {
    // TODO
    // RegisterInputSlot(nsInputSlot_MouseDblClick0, "Left Double Click", nsInputSlotFlags::IsDoubleClick);
    // RegisterInputSlot(nsInputSlot_MouseDblClick1, "Right Double Click", nsInputSlotFlags::IsDoubleClick);
    // RegisterInputSlot(nsInputSlot_MouseDblClick2, "Middle Double Click", nsInputSlotFlags::IsDoubleClick);

    s_iMouseIsOverWindowNumber = 0;
    m_InputSlotValues[nsInputSlot_MousePositionX] = relativePosX;
    m_InputSlotValues[nsInputSlot_MousePositionY] = relativePosY;

    NS_SUCCEED_OR_RETURN(UpdateMouseButtonStates(pointerPoint.Get()));
  }
  else // Touch AND Pen
  {
    // WinRT treats each touch point as unique pointer.
    UINT32 pointerId;
    NS_SUCCEED_OR_RETURN(pointerPoint->get_PointerId(&pointerId));
    if (pointerId > 9)
      return S_OK;

    // All callbacks we subscribed this event to imply that a touch occurs right now.
    m_InputSlotValues[nsInputManager::GetInputSlotTouchPoint(pointerId)] = 1.0f; // Touch strength?
    m_InputSlotValues[nsInputManager::GetInputSlotTouchPointPositionX(pointerId)] = relativePosX;
    m_InputSlotValues[nsInputManager::GetInputSlotTouchPointPositionY(pointerId)] = relativePosY;
  }

  return S_OK;
}

HRESULT nsStandardInputDevice::OnPointerWheelChange(ICoreWindow* coreWindow, IPointerEventArgs* args)
{
  using namespace ABI::Windows::Devices::Input;

  ComPtr<ABI::Windows::UI::Input::IPointerPoint> pointerPoint;
  NS_SUCCEED_OR_RETURN(args->get_CurrentPoint(&pointerPoint));
  ComPtr<IPointerDevice> pointerDevice;
  NS_SUCCEED_OR_RETURN(pointerPoint->get_PointerDevice(&pointerDevice));

  // Only interested in mouse devices.
  PointerDeviceType deviceType;
  NS_SUCCEED_OR_RETURN(pointerDevice->get_PointerDeviceType(&deviceType));
  if (deviceType == PointerDeviceType_Mouse)
  {
    ComPtr<ABI::Windows::UI::Input::IPointerPointProperties> properties;
    NS_SUCCEED_OR_RETURN(pointerPoint->get_Properties(&properties));

    // .. and only vertical wheels.
    boolean isHorizontalWheel;
    NS_SUCCEED_OR_RETURN(properties->get_IsHorizontalMouseWheel(&isHorizontalWheel));
    if (!isHorizontalWheel)
    {
      INT32 delta;
      NS_SUCCEED_OR_RETURN(properties->get_MouseWheelDelta(&delta));

      if (delta > 0)
        m_InputSlotValues[nsInputSlot_MouseWheelUp] = delta / 120.0f;
      else
        m_InputSlotValues[nsInputSlot_MouseWheelDown] = -delta / 120.0f;
    }
  }

  return S_OK;
}

HRESULT nsStandardInputDevice::OnPointerReleasedOrExited(ICoreWindow* coreWindow, IPointerEventArgs* args)
{
  using namespace ABI::Windows::Devices::Input;

  ComPtr<ABI::Windows::UI::Input::IPointerPoint> pointerPoint;
  NS_SUCCEED_OR_RETURN(args->get_CurrentPoint(&pointerPoint));
  ComPtr<IPointerDevice> pointerDevice;
  NS_SUCCEED_OR_RETURN(pointerPoint->get_PointerDevice(&pointerDevice));
  PointerDeviceType deviceType;
  NS_SUCCEED_OR_RETURN(pointerDevice->get_PointerDeviceType(&deviceType));

  if (deviceType == PointerDeviceType_Mouse)
  {
    // Note that the relased event is only fired if the last mouse button is released according to documentation.
    // However, we're also subscribing to exit and depending on the mouse capture this may or may not be a button release.
    NS_SUCCEED_OR_RETURN(UpdateMouseButtonStates(pointerPoint.Get()));
  }
  else // Touch AND Pen
  {
    // WinRT treats each touch point as unique pointer.
    UINT32 pointerId;
    NS_SUCCEED_OR_RETURN(pointerPoint->get_PointerId(&pointerId));
    if (pointerId > 9)
      return S_OK;

    m_InputSlotValues[nsInputManager::GetInputSlotTouchPoint(pointerId)] = 0.0f;
  }

  return S_OK;
}

HRESULT nsStandardInputDevice::OnPointerCaptureLost(ICoreWindow* coreWindow, IPointerEventArgs* args)
{
  using namespace ABI::Windows::Devices::Input;

  ComPtr<ABI::Windows::UI::Input::IPointerPoint> pointerPoint;
  NS_SUCCEED_OR_RETURN(args->get_CurrentPoint(&pointerPoint));
  ComPtr<IPointerDevice> pointerDevice;
  NS_SUCCEED_OR_RETURN(pointerPoint->get_PointerDevice(&pointerDevice));
  PointerDeviceType deviceType;
  NS_SUCCEED_OR_RETURN(pointerDevice->get_PointerDeviceType(&deviceType));

  if (deviceType == PointerDeviceType_Mouse)
  {
    m_InputSlotValues[nsInputSlot_MouseButton0] = 0.0f;
    m_InputSlotValues[nsInputSlot_MouseButton1] = 0.0f;
    m_InputSlotValues[nsInputSlot_MouseButton2] = 0.0f;
    m_InputSlotValues[nsInputSlot_MouseButton3] = 0.0f;
    m_InputSlotValues[nsInputSlot_MouseButton4] = 0.0f;
  }
  else // Touch AND Pen
  {
    // WinRT treats each touch point as unique pointer.
    UINT32 pointerId;
    NS_SUCCEED_OR_RETURN(pointerPoint->get_PointerId(&pointerId));
    if (pointerId > 9)
      return S_OK;

    m_InputSlotValues[nsInputManager::GetInputSlotTouchPoint(pointerId)] = 0.0f;
  }

  return S_OK;
}

HRESULT nsStandardInputDevice::OnMouseMoved(ABI::Windows::Devices::Input::IMouseDevice* mouseDevice,
  ABI::Windows::Devices::Input::IMouseEventArgs* args)
{
  ABI::Windows::Devices::Input::MouseDelta mouseDelta;
  NS_SUCCEED_OR_RETURN(args->get_MouseDelta(&mouseDelta));

  m_InputSlotValues[nsInputSlot_MouseMoveNegX] += ((mouseDelta.X < 0) ? static_cast<float>(-mouseDelta.X) : 0.0f) * GetMouseSpeed().x;
  m_InputSlotValues[nsInputSlot_MouseMovePosX] += ((mouseDelta.X > 0) ? static_cast<float>(mouseDelta.X) : 0.0f) * GetMouseSpeed().x;
  m_InputSlotValues[nsInputSlot_MouseMoveNegY] += ((mouseDelta.Y < 0) ? static_cast<float>(-mouseDelta.Y) : 0.0f) * GetMouseSpeed().y;
  m_InputSlotValues[nsInputSlot_MouseMovePosY] += ((mouseDelta.Y > 0) ? static_cast<float>(mouseDelta.Y) : 0.0f) * GetMouseSpeed().y;

  return S_OK;
}

HRESULT nsStandardInputDevice::UpdateMouseButtonStates(ABI::Windows::UI::Input::IPointerPoint* pointerPoint)
{
  ComPtr<ABI::Windows::UI::Input::IPointerPointProperties> properties;
  NS_SUCCEED_OR_RETURN(pointerPoint->get_Properties(&properties));

  boolean isPressed;
  NS_SUCCEED_OR_RETURN(properties->get_IsLeftButtonPressed(&isPressed));
  m_InputSlotValues[nsInputSlot_MouseButton0] = isPressed ? 1.0f : 0.0f;
  NS_SUCCEED_OR_RETURN(properties->get_IsRightButtonPressed(&isPressed));
  m_InputSlotValues[nsInputSlot_MouseButton1] = isPressed ? 1.0f : 0.0f;
  NS_SUCCEED_OR_RETURN(properties->get_IsMiddleButtonPressed(&isPressed));
  m_InputSlotValues[nsInputSlot_MouseButton2] = isPressed ? 1.0f : 0.0f;
  NS_SUCCEED_OR_RETURN(properties->get_IsXButton1Pressed(&isPressed));
  m_InputSlotValues[nsInputSlot_MouseButton3] = isPressed ? 1.0f : 0.0f;
  NS_SUCCEED_OR_RETURN(properties->get_IsXButton2Pressed(&isPressed));
  m_InputSlotValues[nsInputSlot_MouseButton4] = isPressed ? 1.0f : 0.0f;

  return S_OK;
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

  // RegisterInputSlot(nsInputSlot_KeyPrevTrack, "Previous Track", nsInputSlotFlags::IsButton);
  // RegisterInputSlot(nsInputSlot_KeyNextTrack, "Next Track", nsInputSlotFlags::IsButton);
  // RegisterInputSlot(nsInputSlot_KeyPlayPause, "Play / Pause", nsInputSlotFlags::IsButton);
  // RegisterInputSlot(nsInputSlot_KeyStop, "Stop", nsInputSlotFlags::IsButton);
  // RegisterInputSlot(nsInputSlot_KeyVolumeUp, "Volume Up", nsInputSlotFlags::IsButton);
  // RegisterInputSlot(nsInputSlot_KeyVolumeDown, "Volume Down", nsInputSlotFlags::IsButton);
  // RegisterInputSlot(nsInputSlot_KeyMute, "Mute", nsInputSlotFlags::IsButton);

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


  // Not yet supported
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

void SetClipRect(bool bClip, HWND hWnd)
{
  // NOT IMPLEMENTED. TODO
}

void nsStandardInputDevice::SetClipMouseCursor(nsMouseCursorClipMode::Enum mode)
{
  if (m_ClipCursorMode == mode)
    return;

  if (mode != nsMouseCursorClipMode::NoClip)
    m_coreWindow->SetPointerCapture();
  else
    m_coreWindow->ReleasePointerCapture();

  m_ClipCursorMode = mode;
}

void nsStandardInputDevice::SetShowMouseCursor(bool bShow)
{
  if (m_bShowCursor == bShow)
    return;

  // Hide
  if (!bShow)
  {
    // Save cursor to reinstantiate it.
    m_coreWindow->get_PointerCursor(&m_cursorBeforeHide);
    m_coreWindow->put_PointerCursor(nullptr);
  }

  // Show
  else
  {
    NS_ASSERT_DEV(m_cursorBeforeHide, "There should be a ICoreCursor backup that can be put back.");
    m_coreWindow->put_PointerCursor(m_cursorBeforeHide.Get());
  }

  m_bShowCursor = bShow;
}

bool nsStandardInputDevice::GetShowMouseCursor() const
{
  return m_bShowCursor;
}
