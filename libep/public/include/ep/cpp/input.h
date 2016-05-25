#pragma once
#if !defined(_EP_INPUT_HPP)
#define _EP_INPUT_HPP

// TODO Note some enums and structs moved in here so they can be accessed from Libep components. HAL needs to be revisted to work out what should be available to libep

#include "enum.h"

namespace ep
{
EP_ENUM(InputDeviceState,
        Ready,
        Unavailable
);


// controls for devices
EP_ENUM(MouseControls,
        LeftButton,
        MiddleButton,
        RightButton,
        Button4,
        Button5,
        Wheel,
        XDelta,
        YDelta,
        XAbsolute,
        YAbsolute,
        XBrowser,
        YBrowser,
        XScreen,
        YScreen,

        Max
);

EP_ENUM(GamepadControl,
        AxisLX,
        AxisLY,
        AxisRX,
        AxisRY,
        ButtonA,
        ButtonB,
        ButtonX,
        ButtonY,
        ButtonLB,
        ButtonRB,
        ButtonLT,
        ButtonRT,
        ButtonLThumb,
        ButtonRThumb,
        ButtonStart,
        ButtonBack,
        ButtonDUp,
        ButtonDDown,
        ButtonDLeft,
        ButtonDRight,
        Home,

        Max
);

enum class KeyCode : int
{
  Unknown = 0,

  LShift,
  LCtrl,
  LAlt,
  RShift,
  RCtrl,
  RAlt,
  LWin,
  RWin,
  Menu,

  Left,
  Up,
  Right,
  Down,

  Backspace,
  Tab,

  Enter,

  Escape,

  Insert,
  Delete,

  PageUp,
  PageDown,
  End,
  Home,

  PrintScreen,
  Pause,

  CapsLock,
  ScrollLock,
  NumLock,

  Space,

  Semicolon,
  Equals,
  Comma,
  Hyphen,
  Period,
  ForwardSlash,
  Grave,

  LeftBracket,
  BackSlash,
  RightBracket,
  Apostrophe,

  A,
  B,
  C,
  D,
  E,
  F,
  G,
  H,
  I,
  J,
  K,
  L,
  M,
  N,
  O,
  P,
  Q,
  R,
  S,
  T,
  U,
  V,
  W,
  X,
  Y,
  Z,

  _0,
  _1,
  _2,
  _3,
  _4,
  _5,
  _6,
  _7,
  _8,
  _9,

  F1,
  F2,
  F3,
  F4,
  F5,
  F6,
  F7,
  F8,
  F9,
  F10,
  F11,
  F12,

  Numpad0,
  Numpad1,
  Numpad2,
  Numpad3,
  Numpad4,
  Numpad5,
  Numpad6,
  Numpad7,
  Numpad8,
  Numpad9,
  NumpadMultiply,
  NumpadPlus,
  NumpadMinus,
  NumpadDecimal,
  NumpadDivide,
  NumpadEnter,
  NumpadComma,
  NumpadEquals,

  Max
};
// );

// input API
EP_ENUM(InputDevice,
        Keyboard,
        Mouse,
        Gamepad,
      //  TouchScreen,
      //  Accelerometer,
      //  Compass,

        Max
);

struct InputEvent
{
  InputDevice deviceType;
  int deviceId;

  EP_ENUM(EventType,
    Move,
    Key,
    Focus,
  );

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
    bool hasFocus;
  };
};
}
#endif // !defined(_EP_INPUT_HPP)
