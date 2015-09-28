#pragma once
#ifndef EPINPUT_H
#define EPINPUT_H

#include "ep/epplatform.h"

// input API
enum epInputDevice
{
  epID_Keyboard,
  epID_Mouse,
  epID_Gamepad,
//  epID_TouchScreen,
//  epID_Accelerometer,
//  epID_Compass,

  epID_Max
};

enum epInputDeviceState
{
  epIDS_Ready,
  epIDS_Unavailable
};

epInputDeviceState epInput_GetDeviceState(epInputDevice device, int deviceIndex);

// TODO: remove these, input is strictly event driven!
bool epInput_WasPressed(epInputDevice device, int control, int deviceIndex = -1);
bool epInput_WasReleased(epInputDevice device, int control, int deviceIndex = -1);
float epInput_State(epInputDevice device, int control, int deviceIndex = -1);

unsigned int epInput_LockMouseOnButtons(unsigned int buttonBits);

// controls for devices
enum epMouseControls
{
  epMC_LeftButton,
  epMC_MiddleButton,
  epMC_RightButton,
  epMC_Button4,
  epMC_Button5,
  epMC_Wheel,
  epMC_XDelta,
  epMC_YDelta,
  epMC_XAbsolute,
  epMC_YAbsolute,
  epMC_XBrowser,
  epMC_YBrowser,
  epMC_XScreen,
  epMC_YScreen,

  epMC_Max
};

enum epGamepadControl
{
  epGC_AxisLX,
  epGC_AxisLY,
  epGC_AxisRX,
  epGC_AxisRY,
  epGC_ButtonA,
  epGC_ButtonB,
  epGC_ButtonX,
  epGC_ButtonY,
  epGC_ButtonLB,
  epGC_ButtonRB,
  epGC_ButtonLT,
  epGC_ButtonRT,
  epGC_ButtonLThumb,
  epGC_ButtonRThumb,
  epGC_ButtonStart,
  epGC_ButtonBack,
  epGC_ButtonDUp,
  epGC_ButtonDDown,
  epGC_ButtonDLeft,
  epGC_ButtonDRight,
  epGC_Home,

  epGC_Max
};

enum epKeyCode
{
  epKC_Unknown = 0,

  epKC_LShift,
  epKC_LCtrl,
  epKC_LAlt,
  epKC_RShift,
  epKC_RCtrl,
  epKC_RAlt,
  epKC_LWin,
  epKC_RWin,
  epKC_Menu,

  epKC_Left,
  epKC_Up,
  epKC_Right,
  epKC_Down,

  epKC_Backspace,
  epKC_Tab,

  epKC_Enter,

  epKC_Escape,

  epKC_Insert,
  epKC_Delete,

  epKC_PageUp,
  epKC_PageDown,
  epKC_End,
  epKC_Home,

  epKC_PrintScreen,
  epKC_Pause,

  epKC_CapsLock,
  epKC_ScrollLock,
  epKC_NumLock,

  epKC_Space,

  epKC_Semicolon,
  epKC_Equals,
  epKC_Comma,
  epKC_Hyphen,
  epKC_Period,
  epKC_ForwardSlash,
  epKC_Grave,

  epKC_LeftBracket,
  epKC_BackSlash,
  epKC_RightBracket,
  epKC_Apostrophe,

  epKC_A,
  epKC_B,
  epKC_C,
  epKC_D,
  epKC_E,
  epKC_F,
  epKC_G,
  epKC_H,
  epKC_I,
  epKC_J,
  epKC_K,
  epKC_L,
  epKC_M,
  epKC_N,
  epKC_O,
  epKC_P,
  epKC_Q,
  epKC_R,
  epKC_S,
  epKC_T,
  epKC_U,
  epKC_V,
  epKC_W,
  epKC_X,
  epKC_Y,
  epKC_Z,

  epKC_0,
  epKC_1,
  epKC_2,
  epKC_3,
  epKC_4,
  epKC_5,
  epKC_6,
  epKC_7,
  epKC_8,
  epKC_9,

  epKC_F1,
  epKC_F2,
  epKC_F3,
  epKC_F4,
  epKC_F5,
  epKC_F6,
  epKC_F7,
  epKC_F8,
  epKC_F9,
  epKC_F10,
  epKC_F11,
  epKC_F12,

  epKC_Numpad0,
  epKC_Numpad1,
  epKC_Numpad2,
  epKC_Numpad3,
  epKC_Numpad4,
  epKC_Numpad5,
  epKC_Numpad6,
  epKC_Numpad7,
  epKC_Numpad8,
  epKC_Numpad9,
  epKC_NumpadMultiply,
  epKC_NumpadPlus,
  epKC_NumpadMinus,
  epKC_NumpadDecimal,
  epKC_NumpadDivide,
  epKC_NumpadEnter,
  epKC_NumpadComma,
  epKC_NumpadEquals,

  epKC_Max
};



struct epInputEvent
{
  epInputDevice deviceType;
  int deviceId;

  enum EventType
  {
    Move,
    Key,
  };

  struct MoveEvent
  {
    float xDelta;
    float yDelta;

    float xAbsolute;
    float yAbsolute;
  };

  struct KeyEvent
  {
    int key;
    int state;
  };

  EventType eventType;

  union
  {
    MoveEvent move;
    KeyEvent key;
  };
};


inline epKeyCode epInput_AsciiToKeyCode(unsigned char c)
{
  extern unsigned char epAsciiToEPKey[128];
  return (epKeyCode)epAsciiToEPKey[c];
}


// internal
void epInput_Init();
void epInput_Update();

#endif // EPINPUT_H
