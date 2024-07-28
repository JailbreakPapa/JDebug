#pragma once

#include <Core/CoreDLL.h>
#include <Foundation/Types/Bitflags.h>

/// \brief This struct defines the different states a key can be in.
///        All keys always go through the states 'Pressed' and 'Released', even if they are active for only one frame.
///        A key is 'Down' when it is pressed for at least two frames. It is 'Up' when it is not pressed for at least two frames.
struct NS_CORE_DLL nsKeyState
{
  enum Enum
  {
    Up,       ///< Key is not pressed at all.
    Released, ///< Key has just been released this frame.
    Pressed,  ///< Key has just been pressed down this frame.
    Down      ///< Key is pressed down for longer than one frame now.
  };

  /// \brief Computes the new key state from a previous key state and whether it is currently pressed or not.
  static nsKeyState::Enum GetNewKeyState(nsKeyState::Enum prevState, bool bKeyDown);
};

// clang-format off
// off for the entire file

/// \brief These flags are specified when registering an input slot (by a device), to define some capabilities and restrictions of the hardware.
///
/// By default you do not need to use these flags at all. However, when presenting the user with a list of 'possible' buttons to press to map to an
/// action, these flags can be used to filter out unwanted slots.
/// For example you can filter out mouse movements by requiring that the input slot must be pressable or may not represent any axis.
/// You an additionally also use the prefix of the input slot name, to filter out all touch input slots etc. if necessary.
struct nsInputSlotFlags
{
  using StorageType = nsUInt16;

  enum Enum
  {
    None                      = 0,

    ReportsRelativeValues     = NS_BIT(0),  ///< The input slot reports delta values (e.g. a mouse move), instead of absolute values.
    ValueBinaryZeroOrOne      = NS_BIT(1),  ///< The input slot will either be zero or one. Used for all buttons and keys.
    ValueRangeZeroToOne       = NS_BIT(2),  ///< The input slot has analog values between zero and one. Used for analog axis like the xbox triggers or thumb-sticks.
    ValueRangeZeroToInf       = NS_BIT(3),  ///< The input slot has unbounded values larger than zero. Used for all absolute positions, such as the mouse position.
    Pressable                 = NS_BIT(4),  ///< The slot can be pressed (e.g. a key). This is not possible for an axis, such as the mouse or an analog stick.
    Holdable                  = NS_BIT(5),  ///< The user can hold down the key. Possible for buttons, but not for axes or for wheels such as the mouse wheel.
    HalfAxis                  = NS_BIT(6),  ///< The input slot represents one half of the actually possible data. Used for all axes (pos / neg mouse movement, thumb-sticks).
    FullAxis                  = NS_BIT(7),  ///< The input slot represents one full axis. Mostly used for devices that report absolute values, such as the mouse position or touch input positions (values between zero and one) 
    RequiresDeadZone          = NS_BIT(8),  ///< The input slot represents hardware that should use a dead zone, otherwise it might fire prematurely. Mostly used on thumb-sticks and trigger buttons.
    ValuesAreNonContinuous    = NS_BIT(9),  ///< The values of the slot can jump around randomly, ie. the user can input arbitrary values, like the position on a touchpad
    ActivationDependsOnOthers = NS_BIT(10), ///< Whether this slot can be activated depends on whether certain other slots are active. This is the case for touch-points which are numbered depending on how many other touch-points are already active.
    NeverTimeScale            = NS_BIT(11), ///< When this flag is specified, data from the input slot will never be scaled by the input update time difference. Important for mouse deltas and such.


    // Some predefined sets of flags for the most common use cases
    IsButton                  =                         ValueBinaryZeroOrOne | Pressable | Holdable,
    IsMouseWheel              = ReportsRelativeValues | ValueRangeZeroToInf  | Pressable |            HalfAxis |                    NeverTimeScale,
    IsAnalogTrigger           =                         ValueRangeZeroToOne  | Pressable | Holdable | FullAxis | RequiresDeadZone,
    IsMouseAxisPosition       =                         ValueRangeZeroToOne  |                        FullAxis |                    NeverTimeScale,
    IsMouseAxisMove           = ReportsRelativeValues | ValueRangeZeroToInf  |                        HalfAxis |                    NeverTimeScale,
    IsAnalogStick             =                         ValueRangeZeroToOne  |             Holdable | HalfAxis | RequiresDeadZone,
    IsDoubleClick             =                         ValueBinaryZeroOrOne | Pressable |                                          NeverTimeScale,
    IsTouchPosition           =                         ValueRangeZeroToOne  |                        FullAxis |                    NeverTimeScale | ValuesAreNonContinuous,
    IsTouchPoint              =                         ValueBinaryZeroOrOne | Pressable | Holdable |                                                ActivationDependsOnOthers,
    IsDPad                    =                         ValueBinaryZeroOrOne | Pressable | Holdable | HalfAxis,
    IsTrackedValue            =                         ValueRangeZeroToInf  |                        HalfAxis |                    NeverTimeScale | ValuesAreNonContinuous,

    Default                   = None
  };

  struct Bits
  {
    StorageType ReportsRelativeValues     : 1;
    StorageType ValueBinaryZeroOrOne      : 1;
    StorageType ValueRangeZeroToOne       : 1;
    StorageType ValueRangeZeroToInf       : 1;
    StorageType Pressable                 : 1;
    StorageType Holdable                  : 1;
    StorageType HalfAxis                  : 1;
    StorageType FullAxis                  : 1;
    StorageType RequiresDeadZone          : 1;
    StorageType ValuesAreNonContinuous    : 1;
    StorageType ActivationDependsOnOthers : 1;
  };
};

NS_DECLARE_FLAGS_OPERATORS(nsInputSlotFlags);

#define nsInputSlot_None                  ""

//
// Touchpads
//

#define nsInputSlot_TouchPoint0           "touchpoint_0"
#define nsInputSlot_TouchPoint0_PositionX "touchpoint_0_position_x"
#define nsInputSlot_TouchPoint0_PositionY "touchpoint_0_position_y"

#define nsInputSlot_TouchPoint1           "touchpoint_1"
#define nsInputSlot_TouchPoint1_PositionX "touchpoint_1_position_x"
#define nsInputSlot_TouchPoint1_PositionY "touchpoint_1_position_y"

#define nsInputSlot_TouchPoint2           "touchpoint_2"
#define nsInputSlot_TouchPoint2_PositionX "touchpoint_2_position_x"
#define nsInputSlot_TouchPoint2_PositionY "touchpoint_2_position_y"

#define nsInputSlot_TouchPoint3           "touchpoint_3"
#define nsInputSlot_TouchPoint3_PositionX "touchpoint_3_position_x"
#define nsInputSlot_TouchPoint3_PositionY "touchpoint_3_position_y"

#define nsInputSlot_TouchPoint4           "touchpoint_4"
#define nsInputSlot_TouchPoint4_PositionX "touchpoint_4_position_x"
#define nsInputSlot_TouchPoint4_PositionY "touchpoint_4_position_y"

#define nsInputSlot_TouchPoint5           "touchpoint_5"
#define nsInputSlot_TouchPoint5_PositionX "touchpoint_5_position_x"
#define nsInputSlot_TouchPoint5_PositionY "touchpoint_5_position_y"

#define nsInputSlot_TouchPoint6           "touchpoint_6"
#define nsInputSlot_TouchPoint6_PositionX "touchpoint_6_position_x"
#define nsInputSlot_TouchPoint6_PositionY "touchpoint_6_position_y"

#define nsInputSlot_TouchPoint7           "touchpoint_7"
#define nsInputSlot_TouchPoint7_PositionX "touchpoint_7_position_x"
#define nsInputSlot_TouchPoint7_PositionY "touchpoint_7_position_y"

#define nsInputSlot_TouchPoint8           "touchpoint_8"
#define nsInputSlot_TouchPoint8_PositionX "touchpoint_8_position_x"
#define nsInputSlot_TouchPoint8_PositionY "touchpoint_8_position_y"

#define nsInputSlot_TouchPoint9           "touchpoint_9"
#define nsInputSlot_TouchPoint9_PositionX "touchpoint_9_position_x"
#define nsInputSlot_TouchPoint9_PositionY "touchpoint_9_position_y"

//
// Standard Controllers
//

#define nsInputSlot_Controller0_ButtonA         "controller0_button_a"
#define nsInputSlot_Controller0_ButtonB         "controller0_button_b"
#define nsInputSlot_Controller0_ButtonX         "controller0_button_x"
#define nsInputSlot_Controller0_ButtonY         "controller0_button_y"
#define nsInputSlot_Controller0_ButtonStart     "controller0_button_start"
#define nsInputSlot_Controller0_ButtonBack      "controller0_button_back"
#define nsInputSlot_Controller0_LeftShoulder    "controller0_left_shoulder"
#define nsInputSlot_Controller0_RightShoulder   "controller0_right_shoulder"
#define nsInputSlot_Controller0_LeftTrigger     "controller0_left_trigger"
#define nsInputSlot_Controller0_RightTrigger    "controller0_right_trigger"
#define nsInputSlot_Controller0_PadUp           "controller0_pad_up"
#define nsInputSlot_Controller0_PadDown         "controller0_pad_down"
#define nsInputSlot_Controller0_PadLeft         "controller0_pad_left"
#define nsInputSlot_Controller0_PadRight        "controller0_pad_right"
#define nsInputSlot_Controller0_LeftStick       "controller0_left_stick"
#define nsInputSlot_Controller0_RightStick      "controller0_right_stick"
#define nsInputSlot_Controller0_LeftStick_NegX  "controller0_leftstick_negx"
#define nsInputSlot_Controller0_LeftStick_PosX  "controller0_leftstick_posx"
#define nsInputSlot_Controller0_LeftStick_NegY  "controller0_leftstick_negy"
#define nsInputSlot_Controller0_LeftStick_PosY  "controller0_leftstick_posy"
#define nsInputSlot_Controller0_RightStick_NegX "controller0_rightstick_negx"
#define nsInputSlot_Controller0_RightStick_PosX "controller0_rightstick_posx"
#define nsInputSlot_Controller0_RightStick_NegY "controller0_rightstick_negy"
#define nsInputSlot_Controller0_RightStick_PosY "controller0_rightstick_posy"

#define nsInputSlot_Controller1_ButtonA         "controller1_button_a"
#define nsInputSlot_Controller1_ButtonB         "controller1_button_b"
#define nsInputSlot_Controller1_ButtonX         "controller1_button_x"
#define nsInputSlot_Controller1_ButtonY         "controller1_button_y"
#define nsInputSlot_Controller1_ButtonStart     "controller1_button_start"
#define nsInputSlot_Controller1_ButtonBack      "controller1_button_back"
#define nsInputSlot_Controller1_LeftShoulder    "controller1_left_shoulder"
#define nsInputSlot_Controller1_RightShoulder   "controller1_right_shoulder"
#define nsInputSlot_Controller1_LeftTrigger     "controller1_left_trigger"
#define nsInputSlot_Controller1_RightTrigger    "controller1_right_trigger"
#define nsInputSlot_Controller1_PadUp           "controller1_pad_up"
#define nsInputSlot_Controller1_PadDown         "controller1_pad_down"
#define nsInputSlot_Controller1_PadLeft         "controller1_pad_left"
#define nsInputSlot_Controller1_PadRight        "controller1_pad_right"
#define nsInputSlot_Controller1_LeftStick       "controller1_left_stick"
#define nsInputSlot_Controller1_RightStick      "controller1_right_stick"
#define nsInputSlot_Controller1_LeftStick_NegX  "controller1_leftstick_negx"
#define nsInputSlot_Controller1_LeftStick_PosX  "controller1_leftstick_posx"
#define nsInputSlot_Controller1_LeftStick_NegY  "controller1_leftstick_negy"
#define nsInputSlot_Controller1_LeftStick_PosY  "controller1_leftstick_posy"
#define nsInputSlot_Controller1_RightStick_NegX "controller1_rightstick_negx"
#define nsInputSlot_Controller1_RightStick_PosX "controller1_rightstick_posx"
#define nsInputSlot_Controller1_RightStick_NegY "controller1_rightstick_negy"
#define nsInputSlot_Controller1_RightStick_PosY "controller1_rightstick_posy"

#define nsInputSlot_Controller2_ButtonA         "controller2_button_a"
#define nsInputSlot_Controller2_ButtonB         "controller2_button_b"
#define nsInputSlot_Controller2_ButtonX         "controller2_button_x"
#define nsInputSlot_Controller2_ButtonY         "controller2_button_y"
#define nsInputSlot_Controller2_ButtonStart     "controller2_button_start"
#define nsInputSlot_Controller2_ButtonBack      "controller2_button_back"
#define nsInputSlot_Controller2_LeftShoulder    "controller2_left_shoulder"
#define nsInputSlot_Controller2_RightShoulder   "controller2_right_shoulder"
#define nsInputSlot_Controller2_LeftTrigger     "controller2_left_trigger"
#define nsInputSlot_Controller2_RightTrigger    "controller2_right_trigger"
#define nsInputSlot_Controller2_PadUp           "controller2_pad_up"
#define nsInputSlot_Controller2_PadDown         "controller2_pad_down"
#define nsInputSlot_Controller2_PadLeft         "controller2_pad_left"
#define nsInputSlot_Controller2_PadRight        "controller2_pad_right"
#define nsInputSlot_Controller2_LeftStick       "controller2_left_stick"
#define nsInputSlot_Controller2_RightStick      "controller2_right_stick"
#define nsInputSlot_Controller2_LeftStick_NegX  "controller2_leftstick_negx"
#define nsInputSlot_Controller2_LeftStick_PosX  "controller2_leftstick_posx"
#define nsInputSlot_Controller2_LeftStick_NegY  "controller2_leftstick_negy"
#define nsInputSlot_Controller2_LeftStick_PosY  "controller2_leftstick_posy"
#define nsInputSlot_Controller2_RightStick_NegX "controller2_rightstick_negx"
#define nsInputSlot_Controller2_RightStick_PosX "controller2_rightstick_posx"
#define nsInputSlot_Controller2_RightStick_NegY "controller2_rightstick_negy"
#define nsInputSlot_Controller2_RightStick_PosY "controller2_rightstick_posy"

#define nsInputSlot_Controller3_ButtonA         "controller3_button_a"
#define nsInputSlot_Controller3_ButtonB         "controller3_button_b"
#define nsInputSlot_Controller3_ButtonX         "controller3_button_x"
#define nsInputSlot_Controller3_ButtonY         "controller3_button_y"
#define nsInputSlot_Controller3_ButtonStart     "controller3_button_start"
#define nsInputSlot_Controller3_ButtonBack      "controller3_button_back"
#define nsInputSlot_Controller3_LeftShoulder    "controller3_left_shoulder"
#define nsInputSlot_Controller3_RightShoulder   "controller3_right_shoulder"
#define nsInputSlot_Controller3_LeftTrigger     "controller3_left_trigger"
#define nsInputSlot_Controller3_RightTrigger    "controller3_right_trigger"
#define nsInputSlot_Controller3_PadUp           "controller3_pad_up"
#define nsInputSlot_Controller3_PadDown         "controller3_pad_down"
#define nsInputSlot_Controller3_PadLeft         "controller3_pad_left"
#define nsInputSlot_Controller3_PadRight        "controller3_pad_right"
#define nsInputSlot_Controller3_LeftStick       "controller3_left_stick"
#define nsInputSlot_Controller3_RightStick      "controller3_right_stick"
#define nsInputSlot_Controller3_LeftStick_NegX  "controller3_leftstick_negx"
#define nsInputSlot_Controller3_LeftStick_PosX  "controller3_leftstick_posx"
#define nsInputSlot_Controller3_LeftStick_NegY  "controller3_leftstick_negy"
#define nsInputSlot_Controller3_LeftStick_PosY  "controller3_leftstick_posy"
#define nsInputSlot_Controller3_RightStick_NegX "controller3_rightstick_negx"
#define nsInputSlot_Controller3_RightStick_PosX "controller3_rightstick_posx"
#define nsInputSlot_Controller3_RightStick_NegY "controller3_rightstick_negy"
#define nsInputSlot_Controller3_RightStick_PosY "controller3_rightstick_posy"

//
// Keyboard
//

#define nsInputSlot_KeyLeft           "keyboard_left"
#define nsInputSlot_KeyRight          "keyboard_right"
#define nsInputSlot_KeyUp             "keyboard_up"
#define nsInputSlot_KeyDown           "keyboard_down"
#define nsInputSlot_KeyEscape         "keyboard_escape"
#define nsInputSlot_KeySpace          "keyboard_space"
#define nsInputSlot_KeyBackspace      "keyboard_backspace"
#define nsInputSlot_KeyReturn         "keyboard_return"
#define nsInputSlot_KeyTab            "keyboard_tab"
#define nsInputSlot_KeyLeftShift      "keyboard_left_shift"
#define nsInputSlot_KeyRightShift     "keyboard_right_shift"
#define nsInputSlot_KeyLeftCtrl       "keyboard_left_ctrl"
#define nsInputSlot_KeyRightCtrl      "keyboard_right_ctrl"
#define nsInputSlot_KeyLeftAlt        "keyboard_left_alt"
#define nsInputSlot_KeyRightAlt       "keyboard_right_alt"
#define nsInputSlot_KeyLeftWin        "keyboard_left_win"
#define nsInputSlot_KeyRightWin       "keyboard_right_win"
#define nsInputSlot_KeyBracketOpen    "keyboard_bracket_open"
#define nsInputSlot_KeyBracketClose   "keyboard_bracket_close"
#define nsInputSlot_KeySemicolon      "keyboard_semicolon"
#define nsInputSlot_KeyApostrophe     "keyboard_apostrophe"
#define nsInputSlot_KeySlash          "keyboard_slash"
#define nsInputSlot_KeyEquals         "keyboard_equals"
#define nsInputSlot_KeyTilde          "keyboard_tilde"
#define nsInputSlot_KeyHyphen         "keyboard_hyphen"
#define nsInputSlot_KeyComma          "keyboard_comma"
#define nsInputSlot_KeyPeriod         "keyboard_period"
#define nsInputSlot_KeyBackslash      "keyboard_backslash"
#define nsInputSlot_KeyPipe           "keyboard_pipe"
#define nsInputSlot_Key1              "keyboard_1"
#define nsInputSlot_Key2              "keyboard_2"
#define nsInputSlot_Key3              "keyboard_3"
#define nsInputSlot_Key4              "keyboard_4"
#define nsInputSlot_Key5              "keyboard_5"
#define nsInputSlot_Key6              "keyboard_6"
#define nsInputSlot_Key7              "keyboard_7"
#define nsInputSlot_Key8              "keyboard_8"
#define nsInputSlot_Key9              "keyboard_9"
#define nsInputSlot_Key0              "keyboard_0"
#define nsInputSlot_KeyNumpad1        "keyboard_numpad_1"
#define nsInputSlot_KeyNumpad2        "keyboard_numpad_2"
#define nsInputSlot_KeyNumpad3        "keyboard_numpad_3"
#define nsInputSlot_KeyNumpad4        "keyboard_numpad_4"
#define nsInputSlot_KeyNumpad5        "keyboard_numpad_5"
#define nsInputSlot_KeyNumpad6        "keyboard_numpad_6"
#define nsInputSlot_KeyNumpad7        "keyboard_numpad_7"
#define nsInputSlot_KeyNumpad8        "keyboard_numpad_8"
#define nsInputSlot_KeyNumpad9        "keyboard_numpad_9"
#define nsInputSlot_KeyNumpad0        "keyboard_numpad_0"
#define nsInputSlot_KeyA              "keyboard_a"
#define nsInputSlot_KeyB              "keyboard_b"
#define nsInputSlot_KeyC              "keyboard_c"
#define nsInputSlot_KeyD              "keyboard_d"
#define nsInputSlot_KeyE              "keyboard_e"
#define nsInputSlot_KeyF              "keyboard_f"
#define nsInputSlot_KeyG              "keyboard_g"
#define nsInputSlot_KeyH              "keyboard_h"
#define nsInputSlot_KeyI              "keyboard_i"
#define nsInputSlot_KeyJ              "keyboard_j"
#define nsInputSlot_KeyK              "keyboard_k"
#define nsInputSlot_KeyL              "keyboard_l"
#define nsInputSlot_KeyM              "keyboard_m"
#define nsInputSlot_KeyN              "keyboard_n"
#define nsInputSlot_KeyO              "keyboard_o"
#define nsInputSlot_KeyP              "keyboard_p"
#define nsInputSlot_KeyQ              "keyboard_q"
#define nsInputSlot_KeyR              "keyboard_r"
#define nsInputSlot_KeyS              "keyboard_s"
#define nsInputSlot_KeyT              "keyboard_t"
#define nsInputSlot_KeyU              "keyboard_u"
#define nsInputSlot_KeyV              "keyboard_v"
#define nsInputSlot_KeyW              "keyboard_w"
#define nsInputSlot_KeyX              "keyboard_x"
#define nsInputSlot_KeyY              "keyboard_y"
#define nsInputSlot_KeyZ              "keyboard_z"
#define nsInputSlot_KeyF1             "keyboard_f1"
#define nsInputSlot_KeyF2             "keyboard_f2"
#define nsInputSlot_KeyF3             "keyboard_f3"
#define nsInputSlot_KeyF4             "keyboard_f4"
#define nsInputSlot_KeyF5             "keyboard_f5"
#define nsInputSlot_KeyF6             "keyboard_f6"
#define nsInputSlot_KeyF7             "keyboard_f7"
#define nsInputSlot_KeyF8             "keyboard_f8"
#define nsInputSlot_KeyF9             "keyboard_f9"
#define nsInputSlot_KeyF10            "keyboard_f10"
#define nsInputSlot_KeyF11            "keyboard_f11"
#define nsInputSlot_KeyF12            "keyboard_f12"
#define nsInputSlot_KeyHome           "keyboard_home"
#define nsInputSlot_KeyEnd            "keyboard_end"
#define nsInputSlot_KeyDelete         "keyboard_delete"
#define nsInputSlot_KeyInsert         "keyboard_insert"
#define nsInputSlot_KeyPageUp         "keyboard_page_up"
#define nsInputSlot_KeyPageDown       "keyboard_page_down"
#define nsInputSlot_KeyNumLock        "keyboard_numlock"
#define nsInputSlot_KeyNumpadPlus     "keyboard_numpad_plus"
#define nsInputSlot_KeyNumpadMinus    "keyboard_numpad_minus"
#define nsInputSlot_KeyNumpadStar     "keyboard_numpad_star"
#define nsInputSlot_KeyNumpadSlash    "keyboard_numpad_slash"
#define nsInputSlot_KeyNumpadPeriod   "keyboard_numpad_period"
#define nsInputSlot_KeyNumpadEnter    "keyboard_numpad_enter"
#define nsInputSlot_KeyCapsLock       "keyboard_capslock"
#define nsInputSlot_KeyPrint          "keyboard_print"
#define nsInputSlot_KeyScroll         "keyboard_scroll"
#define nsInputSlot_KeyPause          "keyboard_pause"
#define nsInputSlot_KeyApps           "keyboard_apps"
#define nsInputSlot_KeyPrevTrack      "keyboard_prev_track"
#define nsInputSlot_KeyNextTrack      "keyboard_next_track"
#define nsInputSlot_KeyPlayPause      "keyboard_play_pause"
#define nsInputSlot_KeyStop           "keyboard_stop"
#define nsInputSlot_KeyVolumeUp       "keyboard_volume_up"
#define nsInputSlot_KeyVolumeDown     "keyboard_volume_down"
#define nsInputSlot_KeyMute           "keyboard_mute"

//
// Mouse
//

#define nsInputSlot_MouseWheelUp      "mouse_wheel_up"
#define nsInputSlot_MouseWheelDown    "mouse_wheel_down"
#define nsInputSlot_MouseMoveNegX     "mouse_move_negx"
#define nsInputSlot_MouseMovePosX     "mouse_move_posx"
#define nsInputSlot_MouseMoveNegY     "mouse_move_negy"
#define nsInputSlot_MouseMovePosY     "mouse_move_posy"
#define nsInputSlot_MouseButton0      "mouse_button_0"
#define nsInputSlot_MouseButton1      "mouse_button_1"
#define nsInputSlot_MouseButton2      "mouse_button_2"
#define nsInputSlot_MouseButton3      "mouse_button_3"
#define nsInputSlot_MouseButton4      "mouse_button_4"
#define nsInputSlot_MouseDblClick0    "mouse_button_0_doubleclick"
#define nsInputSlot_MouseDblClick1    "mouse_button_1_doubleclick"
#define nsInputSlot_MouseDblClick2    "mouse_button_2_doubleclick"
#define nsInputSlot_MousePositionX    "mouse_position_x"
#define nsInputSlot_MousePositionY    "mouse_position_y"

//
// Spatial Input Data (Tracked Hands or Controllers)
//

#define nsInputSlot_Spatial_Hand0_Tracked "spatial_hand0_tracked"
#define nsInputSlot_Spatial_Hand0_Pressed "spatial_hand0_pressed"
#define nsInputSlot_Spatial_Hand0_PositionPosX "spatial_hand0_position_posx"
#define nsInputSlot_Spatial_Hand0_PositionPosY "spatial_hand0_position_posy"
#define nsInputSlot_Spatial_Hand0_PositionPosZ "spatial_hand0_position_posz"
#define nsInputSlot_Spatial_Hand0_PositionNegX "spatial_hand0_position_negx"
#define nsInputSlot_Spatial_Hand0_PositionNegY "spatial_hand0_position_negy"
#define nsInputSlot_Spatial_Hand0_PositionNegZ "spatial_hand0_position_negz"

#define nsInputSlot_Spatial_Hand1_Tracked "spatial_hand1_tracked"
#define nsInputSlot_Spatial_Hand1_Pressed "spatial_hand1_pressed"
#define nsInputSlot_Spatial_Hand1_PositionPosX "spatial_hand1_position_posx"
#define nsInputSlot_Spatial_Hand1_PositionPosY "spatial_hand1_position_posy"
#define nsInputSlot_Spatial_Hand1_PositionPosZ "spatial_hand1_position_posz"
#define nsInputSlot_Spatial_Hand1_PositionNegX "spatial_hand1_position_negx"
#define nsInputSlot_Spatial_Hand1_PositionNegY "spatial_hand1_position_negy"
#define nsInputSlot_Spatial_Hand1_PositionNegZ "spatial_hand1_position_negz"

#define nsInputSlot_Spatial_Head_PositionPosX "spatial_head_position_posx"
#define nsInputSlot_Spatial_Head_PositionPosY "spatial_head_position_posy"
#define nsInputSlot_Spatial_Head_PositionPosZ "spatial_head_position_posz"
#define nsInputSlot_Spatial_Head_PositionNegX "spatial_head_position_negx"
#define nsInputSlot_Spatial_Head_PositionNegY "spatial_head_position_negy"
#define nsInputSlot_Spatial_Head_PositionNegZ "spatial_head_position_negz"

#define nsInputSlot_Spatial_Head_ForwardPosX "spatial_head_forward_posx"
#define nsInputSlot_Spatial_Head_ForwardPosY "spatial_head_forward_posy"
#define nsInputSlot_Spatial_Head_ForwardPosZ "spatial_head_forward_posz"
#define nsInputSlot_Spatial_Head_ForwardNegX "spatial_head_forward_negx"
#define nsInputSlot_Spatial_Head_ForwardNegY "spatial_head_forward_negy"
#define nsInputSlot_Spatial_Head_ForwardNegZ "spatial_head_forward_negz"

#define nsInputSlot_Spatial_Head_UpPosX "spatial_head_up_posx"
#define nsInputSlot_Spatial_Head_UpPosY "spatial_head_up_posy"
#define nsInputSlot_Spatial_Head_UpPosZ "spatial_head_up_posz"
#define nsInputSlot_Spatial_Head_UpNegX "spatial_head_up_negx"
#define nsInputSlot_Spatial_Head_UpNegY "spatial_head_up_negy"
#define nsInputSlot_Spatial_Head_UpNegZ "spatial_head_up_negz"

