#include <Core/System/Implementation/glfw/InputDevice_glfw.h>
#include <GLFW/glfw3.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsStandardInputDevice, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

namespace
{
  const char* ConvertGLFWKeyToEngineName(int key)
  {
    switch (key)
    {
      case GLFW_KEY_LEFT:
        return nsInputSlot_KeyLeft;
      case GLFW_KEY_RIGHT:
        return nsInputSlot_KeyRight;
      case GLFW_KEY_UP:
        return nsInputSlot_KeyUp;
      case GLFW_KEY_DOWN:
        return nsInputSlot_KeyDown;
      case GLFW_KEY_ESCAPE:
        return nsInputSlot_KeyEscape;
      case GLFW_KEY_SPACE:
        return nsInputSlot_KeySpace;
      case GLFW_KEY_BACKSPACE:
        return nsInputSlot_KeyBackspace;
      case GLFW_KEY_ENTER:
        return nsInputSlot_KeyReturn;
      case GLFW_KEY_TAB:
        return nsInputSlot_KeyTab;
      case GLFW_KEY_LEFT_SHIFT:
        return nsInputSlot_KeyLeftShift;
      case GLFW_KEY_RIGHT_SHIFT:
        return nsInputSlot_KeyRightShift;
      case GLFW_KEY_LEFT_CONTROL:
        return nsInputSlot_KeyLeftCtrl;
      case GLFW_KEY_RIGHT_CONTROL:
        return nsInputSlot_KeyRightCtrl;
      case GLFW_KEY_LEFT_ALT:
        return nsInputSlot_KeyLeftAlt;
      case GLFW_KEY_RIGHT_ALT:
        return nsInputSlot_KeyRightAlt;
      case GLFW_KEY_LEFT_SUPER:
        return nsInputSlot_KeyLeftWin;
      case GLFW_KEY_RIGHT_SUPER:
        return nsInputSlot_KeyRightWin;
      case GLFW_KEY_MENU:
        return nsInputSlot_KeyApps;
      case GLFW_KEY_LEFT_BRACKET:
        return nsInputSlot_KeyBracketOpen;
      case GLFW_KEY_RIGHT_BRACKET:
        return nsInputSlot_KeyBracketClose;
      case GLFW_KEY_SEMICOLON:
        return nsInputSlot_KeySemicolon;
      case GLFW_KEY_APOSTROPHE:
        return nsInputSlot_KeyApostrophe;
      case GLFW_KEY_SLASH:
        return nsInputSlot_KeySlash;
      case GLFW_KEY_EQUAL:
        return nsInputSlot_KeyEquals;
      case GLFW_KEY_GRAVE_ACCENT:
        return nsInputSlot_KeyTilde;
      case GLFW_KEY_MINUS:
        return nsInputSlot_KeyHyphen;
      case GLFW_KEY_COMMA:
        return nsInputSlot_KeyComma;
      case GLFW_KEY_PERIOD:
        return nsInputSlot_KeyPeriod;
      case GLFW_KEY_BACKSLASH:
        return nsInputSlot_KeyBackslash;
      case GLFW_KEY_WORLD_1:
        return nsInputSlot_KeyPipe;
      case GLFW_KEY_1:
        return nsInputSlot_Key1;
      case GLFW_KEY_2:
        return nsInputSlot_Key2;
      case GLFW_KEY_3:
        return nsInputSlot_Key3;
      case GLFW_KEY_4:
        return nsInputSlot_Key4;
      case GLFW_KEY_5:
        return nsInputSlot_Key5;
      case GLFW_KEY_6:
        return nsInputSlot_Key6;
      case GLFW_KEY_7:
        return nsInputSlot_Key7;
      case GLFW_KEY_8:
        return nsInputSlot_Key8;
      case GLFW_KEY_9:
        return nsInputSlot_Key9;
      case GLFW_KEY_0:
        return nsInputSlot_Key0;
      case GLFW_KEY_KP_1:
        return nsInputSlot_KeyNumpad1;
      case GLFW_KEY_KP_2:
        return nsInputSlot_KeyNumpad2;
      case GLFW_KEY_KP_3:
        return nsInputSlot_KeyNumpad3;
      case GLFW_KEY_KP_4:
        return nsInputSlot_KeyNumpad4;
      case GLFW_KEY_KP_5:
        return nsInputSlot_KeyNumpad5;
      case GLFW_KEY_KP_6:
        return nsInputSlot_KeyNumpad6;
      case GLFW_KEY_KP_7:
        return nsInputSlot_KeyNumpad7;
      case GLFW_KEY_KP_8:
        return nsInputSlot_KeyNumpad8;
      case GLFW_KEY_KP_9:
        return nsInputSlot_KeyNumpad9;
      case GLFW_KEY_KP_0:
        return nsInputSlot_KeyNumpad0;
      case GLFW_KEY_A:
        return nsInputSlot_KeyA;
      case GLFW_KEY_B:
        return nsInputSlot_KeyB;
      case GLFW_KEY_C:
        return nsInputSlot_KeyC;
      case GLFW_KEY_D:
        return nsInputSlot_KeyD;
      case GLFW_KEY_E:
        return nsInputSlot_KeyE;
      case GLFW_KEY_F:
        return nsInputSlot_KeyF;
      case GLFW_KEY_G:
        return nsInputSlot_KeyG;
      case GLFW_KEY_H:
        return nsInputSlot_KeyH;
      case GLFW_KEY_I:
        return nsInputSlot_KeyI;
      case GLFW_KEY_J:
        return nsInputSlot_KeyJ;
      case GLFW_KEY_K:
        return nsInputSlot_KeyK;
      case GLFW_KEY_L:
        return nsInputSlot_KeyL;
      case GLFW_KEY_M:
        return nsInputSlot_KeyM;
      case GLFW_KEY_N:
        return nsInputSlot_KeyN;
      case GLFW_KEY_O:
        return nsInputSlot_KeyO;
      case GLFW_KEY_P:
        return nsInputSlot_KeyP;
      case GLFW_KEY_Q:
        return nsInputSlot_KeyQ;
      case GLFW_KEY_R:
        return nsInputSlot_KeyR;
      case GLFW_KEY_S:
        return nsInputSlot_KeyS;
      case GLFW_KEY_T:
        return nsInputSlot_KeyT;
      case GLFW_KEY_U:
        return nsInputSlot_KeyU;
      case GLFW_KEY_V:
        return nsInputSlot_KeyV;
      case GLFW_KEY_W:
        return nsInputSlot_KeyW;
      case GLFW_KEY_X:
        return nsInputSlot_KeyX;
      case GLFW_KEY_Y:
        return nsInputSlot_KeyY;
      case GLFW_KEY_Z:
        return nsInputSlot_KeyZ;
      case GLFW_KEY_F1:
        return nsInputSlot_KeyF1;
      case GLFW_KEY_F2:
        return nsInputSlot_KeyF2;
      case GLFW_KEY_F3:
        return nsInputSlot_KeyF3;
      case GLFW_KEY_F4:
        return nsInputSlot_KeyF4;
      case GLFW_KEY_F5:
        return nsInputSlot_KeyF5;
      case GLFW_KEY_F6:
        return nsInputSlot_KeyF6;
      case GLFW_KEY_F7:
        return nsInputSlot_KeyF7;
      case GLFW_KEY_F8:
        return nsInputSlot_KeyF8;
      case GLFW_KEY_F9:
        return nsInputSlot_KeyF9;
      case GLFW_KEY_F10:
        return nsInputSlot_KeyF10;
      case GLFW_KEY_F11:
        return nsInputSlot_KeyF11;
      case GLFW_KEY_F12:
        return nsInputSlot_KeyF12;
      case GLFW_KEY_HOME:
        return nsInputSlot_KeyHome;
      case GLFW_KEY_END:
        return nsInputSlot_KeyEnd;
      case GLFW_KEY_DELETE:
        return nsInputSlot_KeyDelete;
      case GLFW_KEY_INSERT:
        return nsInputSlot_KeyInsert;
      case GLFW_KEY_PAGE_UP:
        return nsInputSlot_KeyPageUp;
      case GLFW_KEY_PAGE_DOWN:
        return nsInputSlot_KeyPageDown;
      case GLFW_KEY_NUM_LOCK:
        return nsInputSlot_KeyNumLock;
      case GLFW_KEY_KP_ADD:
        return nsInputSlot_KeyNumpadPlus;
      case GLFW_KEY_KP_SUBTRACT:
        return nsInputSlot_KeyNumpadMinus;
      case GLFW_KEY_KP_MULTIPLY:
        return nsInputSlot_KeyNumpadStar;
      case GLFW_KEY_KP_DIVIDE:
        return nsInputSlot_KeyNumpadSlash;
      case GLFW_KEY_KP_DECIMAL:
        return nsInputSlot_KeyNumpadPeriod;
      case GLFW_KEY_KP_ENTER:
        return nsInputSlot_KeyNumpadEnter;
      case GLFW_KEY_CAPS_LOCK:
        return nsInputSlot_KeyCapsLock;
      case GLFW_KEY_PRINT_SCREEN:
        return nsInputSlot_KeyPrint;
      case GLFW_KEY_SCROLL_LOCK:
        return nsInputSlot_KeyScroll;
      case GLFW_KEY_PAUSE:
        return nsInputSlot_KeyPause;
      // TODO nsInputSlot_KeyPrevTrack
      // TODO nsInputSlot_KeyNextTrack
      // TODO nsInputSlot_KeyPlayPause
      // TODO nsInputSlot_KeyStop
      // TODO nsInputSlot_KeyVolumeUp
      // TODO nsInputSlot_KeyVolumeDown
      // TODO nsInputSlot_KeyMute
      default:
        return nullptr;
    }
  }
} // namespace

nsStandardInputDevice::nsStandardInputDevice(nsUInt32 uiWindowNumber, GLFWwindow* windowHandle)
  : m_uiWindowNumber(uiWindowNumber)
  , m_pWindow(windowHandle)
{
}

nsStandardInputDevice::~nsStandardInputDevice()
{
}

void nsStandardInputDevice::SetShowMouseCursor(bool bShow)
{
  glfwSetInputMode(m_pWindow, GLFW_CURSOR, bShow ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}

bool nsStandardInputDevice::GetShowMouseCursor() const
{
  return (glfwGetInputMode(m_pWindow, GLFW_CURSOR) != GLFW_CURSOR_DISABLED);
}

void nsStandardInputDevice::SetClipMouseCursor(nsMouseCursorClipMode::Enum mode)
{
}

nsMouseCursorClipMode::Enum nsStandardInputDevice::GetClipMouseCursor() const
{
  return nsMouseCursorClipMode::Default;
}

void nsStandardInputDevice::InitializeDevice() {}

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
  // TODO RegisterInputSlot(nsInputSlot_KeyTilde, "~", nsInputSlotFlags::IsButton);
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

  /* TODO
  RegisterInputSlot(nsInputSlot_KeyPrevTrack, "Previous Track", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyNextTrack, "Next Track", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyPlayPause, "Play / Pause", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyStop, "Stop", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyVolumeUp, "Volume Up", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyVolumeDown, "Volume Down", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_KeyMute, "Mute", nsInputSlotFlags::IsButton);
  */

  RegisterInputSlot(nsInputSlot_MousePositionX, "Mouse Position X", nsInputSlotFlags::IsMouseAxisPosition);
  RegisterInputSlot(nsInputSlot_MousePositionY, "Mouse Position Y", nsInputSlotFlags::IsMouseAxisPosition);

  RegisterInputSlot(nsInputSlot_MouseMoveNegX, "Mouse Move Left", nsInputSlotFlags::IsMouseAxisMove);
  RegisterInputSlot(nsInputSlot_MouseMovePosX, "Mouse Move Right", nsInputSlotFlags::IsMouseAxisMove);
  RegisterInputSlot(nsInputSlot_MouseMoveNegY, "Mouse Move Down", nsInputSlotFlags::IsMouseAxisMove);
  RegisterInputSlot(nsInputSlot_MouseMovePosY, "Mouse Move Up", nsInputSlotFlags::IsMouseAxisMove);

  RegisterInputSlot(nsInputSlot_MouseButton0, "Mousebutton 0", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_MouseButton1, "Mousebutton 1", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_MouseButton2, "Mousebutton 2", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_MouseButton3, "Mousebutton 3", nsInputSlotFlags::IsButton);
  RegisterInputSlot(nsInputSlot_MouseButton4, "Mousebutton 4", nsInputSlotFlags::IsButton);

  RegisterInputSlot(nsInputSlot_MouseWheelUp, "Mousewheel Up", nsInputSlotFlags::IsMouseWheel);
  RegisterInputSlot(nsInputSlot_MouseWheelDown, "Mousewheel Down", nsInputSlotFlags::IsMouseWheel);
}

void nsStandardInputDevice::ResetInputSlotValues()
{
  m_InputSlotValues[nsInputSlot_MouseWheelUp] = 0;
  m_InputSlotValues[nsInputSlot_MouseWheelDown] = 0;
  m_InputSlotValues[nsInputSlot_MouseMoveNegX] = 0;
  m_InputSlotValues[nsInputSlot_MouseMovePosX] = 0;
  m_InputSlotValues[nsInputSlot_MouseMoveNegY] = 0;
  m_InputSlotValues[nsInputSlot_MouseMovePosY] = 0;
}

void nsStandardInputDevice::OnKey(int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_BACKSPACE && (action == GLFW_PRESS || action == GLFW_REPEAT))
  {
    m_uiLastCharacter = 0x00000008;
  }

  const char* szInputSlotName = ConvertGLFWKeyToEngineName(key);
  if (szInputSlotName)
  {
    m_InputSlotValues[szInputSlotName] = (action == GLFW_RELEASE) ? 0.0f : 1.0f;
  }
  else
  {
    nsLog::Warning("Unhandeled glfw keyboard key {} {}", key, (action == GLFW_RELEASE) ? "released" : "pressed");
  }
}

void nsStandardInputDevice::OnCharacter(unsigned int codepoint)
{
  m_uiLastCharacter = codepoint;
}

void nsStandardInputDevice::OnCursorPosition(double xpos, double ypos)
{
  s_iMouseIsOverWindowNumber = m_uiWindowNumber;

  int width;
  int height;
  glfwGetWindowSize(m_pWindow, &width, &height);

  m_InputSlotValues[nsInputSlot_MousePositionX] = static_cast<float>(xpos / width);
  m_InputSlotValues[nsInputSlot_MousePositionY] = static_cast<float>(ypos / height);

  if (m_LastPos.x != nsMath::MaxValue<double>())
  {
    nsVec2d diff = nsVec2d(xpos, ypos) - m_LastPos;

    m_InputSlotValues[nsInputSlot_MouseMoveNegX] += ((diff.x < 0) ? (float)-diff.x : 0.0f) * GetMouseSpeed().x;
    m_InputSlotValues[nsInputSlot_MouseMovePosX] += ((diff.x > 0) ? (float)diff.x : 0.0f) * GetMouseSpeed().x;
    m_InputSlotValues[nsInputSlot_MouseMoveNegY] += ((diff.y < 0) ? (float)-diff.y : 0.0f) * GetMouseSpeed().y;
    m_InputSlotValues[nsInputSlot_MouseMovePosY] += ((diff.y > 0) ? (float)diff.y : 0.0f) * GetMouseSpeed().y;
  }
  m_LastPos = nsVec2d(xpos, ypos);
}

void nsStandardInputDevice::OnMouseButton(int button, int action, int mods)
{
  const char* inputSlot = nullptr;
  switch (button)
  {
    case GLFW_MOUSE_BUTTON_1:
      inputSlot = nsInputSlot_MouseButton0;
      break;
    case GLFW_MOUSE_BUTTON_2:
      inputSlot = nsInputSlot_MouseButton1;
      break;
    case GLFW_MOUSE_BUTTON_3:
      inputSlot = nsInputSlot_MouseButton2;
      break;
    case GLFW_MOUSE_BUTTON_4:
      inputSlot = nsInputSlot_MouseButton3;
      break;
    case GLFW_MOUSE_BUTTON_5:
      inputSlot = nsInputSlot_MouseButton4;
      break;
  }

  if (inputSlot)
  {
    m_InputSlotValues[inputSlot] = (action == GLFW_PRESS) ? 1.0f : 0.0f;
  }
}

void nsStandardInputDevice::OnScroll(double xoffset, double yoffset)
{
  if (yoffset > 0)
  {
    m_InputSlotValues[nsInputSlot_MouseWheelUp] = static_cast<float>(yoffset);
  }
  else
  {
    m_InputSlotValues[nsInputSlot_MouseWheelDown] = static_cast<float>(-yoffset);
  }
}
