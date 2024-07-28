#pragma once

#include <Core/Input/InputDevice.h>
#include <Foundation/Math/Vec2.h>

/// \brief Specifies how to restrict movement of the Operating System mouse
struct nsMouseCursorClipMode
{
  enum Enum
  {
    NoClip,                ///< The mouse can move unrestricted and leave the application window
    ClipToWindow,          ///< The mouse cannot leave the window area anymore after the user started interacting with it (ie. clicks into the
                           ///< window)
    ClipToWindowImmediate, ///< The mouse gets restricted to the window area as soon as possible
    ClipToPosition,        ///< The mouse may not leave its current position. Can be used to keep the mouse in place while it is hidden. Note that
                           ///< you will still get mouse move deltas, just the OS cursor will stay in place.

    Default = NoClip,
  };
};

/// \brief This is the base class for all input devices that handle mouse and keyboard input.
///
/// This class is derived from nsInputDevice but adds interface functions to handle mouse and keyboard input.
class NS_CORE_DLL nsInputDeviceMouseKeyboard : public nsInputDevice
{
  NS_ADD_DYNAMIC_REFLECTION(nsInputDeviceMouseKeyboard, nsInputDevice);

public:
  nsInputDeviceMouseKeyboard() { m_vMouseScale.Set(1.0f); }

  /// \brief Shows or hides the mouse cursor inside the application window.
  virtual void SetShowMouseCursor(bool bShow) = 0;

  /// \brief Returns whether the mouse cursor is shown.
  virtual bool GetShowMouseCursor() const = 0;

  /// \brief Will trap the mouse inside the application window. Should usually be enabled, to prevent accidental task switches.
  ///
  /// Especially on multi-monitor systems, the mouse can easily leave the application window (even in fullscreen mode).
  /// Do NOT use this function when you have multiple windows and require absolute mouse positions.
  ///
  /// \sa nsMouseCursorClipMode
  virtual void SetClipMouseCursor(nsMouseCursorClipMode::Enum mode) = 0;

  /// \brief Returns whether the mouse is confined to the application window or not.
  virtual nsMouseCursorClipMode::Enum GetClipMouseCursor() const = 0;

  /// \brief Sets the scaling factor that is applied on all (relative) mouse input.
  virtual void SetMouseSpeed(const nsVec2& vScale) { m_vMouseScale = vScale; }

  /// \brief Returns the scaling factor that is applied on all (relative) mouse input.
  nsVec2 GetMouseSpeed() const { return m_vMouseScale; }

  /// \brief Returns the number of the nsWindow over which the mouse moved last.
  static nsInt32 GetWindowNumberMouseIsOver() { return s_iMouseIsOverWindowNumber; }

  /// \brief Returns if the associated nsWindow has focus
  bool IsFocused() { return m_bIsFocused; }

protected:
  virtual void UpdateInputSlotValues() override;

  nsTime m_DoubleClickTime = nsTime::MakeFromMilliseconds(500);
  static nsInt32 s_iMouseIsOverWindowNumber;

private:
  nsVec2 m_vMouseScale;

  bool m_bIsFocused = true;

  nsTime m_LastMouseClick[3];
  bool m_bMouseDown[3] = {false, false, false};
};
