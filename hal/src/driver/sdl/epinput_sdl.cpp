#include "driver.h"

#if EPINPUT_DRIVER == EPDRIVER_SDL

#include "input_internal.h"

#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_gamecontroller.h>

using namespace ep;

static unsigned char sdlScanCodeToEPKey[] =
{
  (unsigned char)KeyCode::Unknown, // 0     SDL_SCANCODE_UNKNOWN,
  (unsigned char)KeyCode::Unknown, // 1
  (unsigned char)KeyCode::Unknown, // 2
  (unsigned char)KeyCode::Unknown, // 3
  (unsigned char)KeyCode::A, // 4     SDL_SCANCODE_A,
  (unsigned char)KeyCode::B, // 5     SDL_SCANCODE_B,
  (unsigned char)KeyCode::C, // 6     SDL_SCANCODE_C,
  (unsigned char)KeyCode::D, // 7     SDL_SCANCODE_D,
  (unsigned char)KeyCode::E, // 8     SDL_SCANCODE_E,
  (unsigned char)KeyCode::F, // 9     SDL_SCANCODE_F,
  (unsigned char)KeyCode::G, // 10    SDL_SCANCODE_G,
  (unsigned char)KeyCode::H, // 11    SDL_SCANCODE_H,
  (unsigned char)KeyCode::I, // 12    SDL_SCANCODE_I,
  (unsigned char)KeyCode::J, // 13    SDL_SCANCODE_J,
  (unsigned char)KeyCode::K, // 14    SDL_SCANCODE_K,
  (unsigned char)KeyCode::L, // 15    SDL_SCANCODE_L,
  (unsigned char)KeyCode::M, // 16    SDL_SCANCODE_M,
  (unsigned char)KeyCode::N, // 17    SDL_SCANCODE_N,
  (unsigned char)KeyCode::O, // 18    SDL_SCANCODE_O,
  (unsigned char)KeyCode::P, // 19    SDL_SCANCODE_P,
  (unsigned char)KeyCode::Q, // 20    SDL_SCANCODE_Q,
  (unsigned char)KeyCode::R, // 21    SDL_SCANCODE_R,
  (unsigned char)KeyCode::S, // 22    SDL_SCANCODE_S,
  (unsigned char)KeyCode::T, // 23    SDL_SCANCODE_T,
  (unsigned char)KeyCode::U, // 24    SDL_SCANCODE_U,
  (unsigned char)KeyCode::V, // 25    SDL_SCANCODE_V,
  (unsigned char)KeyCode::W, // 26    SDL_SCANCODE_W,
  (unsigned char)KeyCode::X, // 27    SDL_SCANCODE_X,
  (unsigned char)KeyCode::Y, // 28    SDL_SCANCODE_Y,
  (unsigned char)KeyCode::Z, // 29    SDL_SCANCODE_Z,
  (unsigned char)KeyCode::_1, // 30    SDL_SCANCODE_1,
  (unsigned char)KeyCode::_2, // 31    SDL_SCANCODE_2,
  (unsigned char)KeyCode::_3, // 32    SDL_SCANCODE_3,
  (unsigned char)KeyCode::_4, // 33    SDL_SCANCODE_4,
  (unsigned char)KeyCode::_5, // 34    SDL_SCANCODE_5,
  (unsigned char)KeyCode::_6, // 35    SDL_SCANCODE_6,
  (unsigned char)KeyCode::_7, // 36    SDL_SCANCODE_7,
  (unsigned char)KeyCode::_8, // 37    SDL_SCANCODE_8,
  (unsigned char)KeyCode::_9, // 38    SDL_SCANCODE_9,
  (unsigned char)KeyCode::_0, // 39    SDL_SCANCODE_0,
  (unsigned char)KeyCode::Enter, // 40    SDL_SCANCODE_RETURN,
  (unsigned char)KeyCode::Escape, // 41    SDL_SCANCODE_ESCAPE,
  (unsigned char)KeyCode::Backspace, // 42    SDL_SCANCODE_BACKSPACE,
  (unsigned char)KeyCode::Tab, // 43    SDL_SCANCODE_TAB,
  (unsigned char)KeyCode::Space, // 44    SDL_SCANCODE_SPACE,
  (unsigned char)KeyCode::Hyphen, // 45    SDL_SCANCODE_MINUS,
  (unsigned char)KeyCode::Equals, // 46    SDL_SCANCODE_EQUALS,
  (unsigned char)KeyCode::LeftBracket, // 47    SDL_SCANCODE_LEFTBRACKET,
  (unsigned char)KeyCode::RightBracket, // 48    SDL_SCANCODE_RIGHTBRACKET,
  (unsigned char)KeyCode::BackSlash, // 49    SDL_SCANCODE_BACKSLASH,
  (unsigned char)KeyCode::Unknown, // 50    SDL_SCANCODE_NONUSHASH,
  (unsigned char)KeyCode::Semicolon, // 51    SDL_SCANCODE_SEMICOLON,
  (unsigned char)KeyCode::Apostrophe, // 52    SDL_SCANCODE_APOSTROPHE,
  (unsigned char)KeyCode::Grave, // 53    SDL_SCANCODE_GRAVE,
  (unsigned char)KeyCode::Comma, // 54    SDL_SCANCODE_COMMA,
  (unsigned char)KeyCode::Period, // 55    SDL_SCANCODE_PERIOD,
  (unsigned char)KeyCode::ForwardSlash, // 56    SDL_SCANCODE_SLASH,
  (unsigned char)KeyCode::CapsLock, // 57    SDL_SCANCODE_CAPSLOCK,
  (unsigned char)KeyCode::F1, // 58    SDL_SCANCODE_F1,
  (unsigned char)KeyCode::F2, // 59    SDL_SCANCODE_F2,
  (unsigned char)KeyCode::F3, // 60    SDL_SCANCODE_F3,
  (unsigned char)KeyCode::F4, // 61    SDL_SCANCODE_F4,
  (unsigned char)KeyCode::F5, // 62    SDL_SCANCODE_F5,
  (unsigned char)KeyCode::F6, // 63    SDL_SCANCODE_F6,
  (unsigned char)KeyCode::F7, // 64    SDL_SCANCODE_F7,
  (unsigned char)KeyCode::F8, // 65    SDL_SCANCODE_F8,
  (unsigned char)KeyCode::F9, // 66    SDL_SCANCODE_F9,
  (unsigned char)KeyCode::F10, // 67    SDL_SCANCODE_F10,
  (unsigned char)KeyCode::F11, // 68    SDL_SCANCODE_F11,
  (unsigned char)KeyCode::F12, // 69    SDL_SCANCODE_F12,
  (unsigned char)KeyCode::PrintScreen, // 70    SDL_SCANCODE_PRINTSCREEN,
  (unsigned char)KeyCode::ScrollLock, // 71    SDL_SCANCODE_SCROLLLOCK,
  (unsigned char)KeyCode::Pause, // 72    SDL_SCANCODE_PAUSE,
  (unsigned char)KeyCode::Insert, // 73    SDL_SCANCODE_INSERT,
  (unsigned char)KeyCode::Home, // 74    SDL_SCANCODE_HOME,
  (unsigned char)KeyCode::PageUp, // 75    SDL_SCANCODE_PAGEUP,
  (unsigned char)KeyCode::Delete, // 76    SDL_SCANCODE_DELETE,
  (unsigned char)KeyCode::End, // 77    SDL_SCANCODE_END,
  (unsigned char)KeyCode::PageDown, // 78    SDL_SCANCODE_PAGEDOWN,
  (unsigned char)KeyCode::Right, // 79    SDL_SCANCODE_RIGHT,
  (unsigned char)KeyCode::Left, // 80    SDL_SCANCODE_LEFT,
  (unsigned char)KeyCode::Down, // 81    SDL_SCANCODE_DOWN,
  (unsigned char)KeyCode::Up, // 82    SDL_SCANCODE_UP,
  (unsigned char)KeyCode::NumLock, // 83    SDL_SCANCODE_NUMLOCKCLEAR,
  (unsigned char)KeyCode::NumpadDivide, // 84    SDL_SCANCODE_KP_DIVIDE,
  (unsigned char)KeyCode::NumpadMultiply, // 85    SDL_SCANCODE_KP_MULTIPLY,
  (unsigned char)KeyCode::NumpadMinus, // 86    SDL_SCANCODE_KP_MINUS,
  (unsigned char)KeyCode::NumpadPlus, // 87    SDL_SCANCODE_KP_PLUS,
  (unsigned char)KeyCode::NumpadEnter, // 88    SDL_SCANCODE_KP_ENTER,
  (unsigned char)KeyCode::Numpad1, // 89    SDL_SCANCODE_KP_1,
  (unsigned char)KeyCode::Numpad2, // 90    SDL_SCANCODE_KP_2,
  (unsigned char)KeyCode::Numpad3, // 91    SDL_SCANCODE_KP_3,
  (unsigned char)KeyCode::Numpad4, // 92    SDL_SCANCODE_KP_4,
  (unsigned char)KeyCode::Numpad5, // 93    SDL_SCANCODE_KP_5,
  (unsigned char)KeyCode::Numpad6, // 94    SDL_SCANCODE_KP_6,
  (unsigned char)KeyCode::Numpad7, // 95    SDL_SCANCODE_KP_7,
  (unsigned char)KeyCode::Numpad8, // 96    SDL_SCANCODE_KP_8,
  (unsigned char)KeyCode::Numpad9, // 97    SDL_SCANCODE_KP_9,
  (unsigned char)KeyCode::Numpad0, // 98    SDL_SCANCODE_KP_0,
  (unsigned char)KeyCode::NumpadDecimal, // 99    SDL_SCANCODE_KP_PERIOD,
  (unsigned char)KeyCode::Unknown, // 100   SDL_SCANCODE_NONUSBACKSLASH,
  (unsigned char)KeyCode::Unknown, // 101   SDL_SCANCODE_APPLICATION,
  (unsigned char)KeyCode::Unknown, // 102   SDL_SCANCODE_POWER,
  (unsigned char)KeyCode::NumpadEquals, // 103   SDL_SCANCODE_KP_EQUALS,
  (unsigned char)KeyCode::Unknown, // 104   SDL_SCANCODE_F13,
  (unsigned char)KeyCode::Unknown, // 105   SDL_SCANCODE_F14,
  (unsigned char)KeyCode::Unknown, // 106   SDL_SCANCODE_F15,
  (unsigned char)KeyCode::Unknown, // 107   SDL_SCANCODE_F16,
  (unsigned char)KeyCode::Unknown, // 108   SDL_SCANCODE_F17,
  (unsigned char)KeyCode::Unknown, // 109   SDL_SCANCODE_F18,
  (unsigned char)KeyCode::Unknown, // 110   SDL_SCANCODE_F19,
  (unsigned char)KeyCode::Unknown, // 111   SDL_SCANCODE_F20,
  (unsigned char)KeyCode::Unknown, // 112   SDL_SCANCODE_F21,
  (unsigned char)KeyCode::Unknown, // 113   SDL_SCANCODE_F22,
  (unsigned char)KeyCode::Unknown, // 114   SDL_SCANCODE_F23,
  (unsigned char)KeyCode::Unknown, // 115   SDL_SCANCODE_F24,
  (unsigned char)KeyCode::Unknown, // 116   SDL_SCANCODE_EXECUTE,
  (unsigned char)KeyCode::Unknown, // 117   SDL_SCANCODE_HELP,
  (unsigned char)KeyCode::Menu, // 118   SDL_SCANCODE_MENU,
  (unsigned char)KeyCode::Unknown, // 119   SDL_SCANCODE_SELECT,
  (unsigned char)KeyCode::Unknown, // 120   SDL_SCANCODE_STOP,
  (unsigned char)KeyCode::Unknown, // 121   SDL_SCANCODE_AGAIN,
  (unsigned char)KeyCode::Unknown, // 122   SDL_SCANCODE_UNDO,
  (unsigned char)KeyCode::Unknown, // 123   SDL_SCANCODE_CUT,
  (unsigned char)KeyCode::Unknown, // 124   SDL_SCANCODE_COPY,
  (unsigned char)KeyCode::Unknown, // 125   SDL_SCANCODE_PASTE,
  (unsigned char)KeyCode::Unknown, // 126   SDL_SCANCODE_FIND,
  (unsigned char)KeyCode::Unknown, // 127   SDL_SCANCODE_MUTE,
  (unsigned char)KeyCode::Unknown, // 128   SDL_SCANCODE_VOLUMEUP,
  (unsigned char)KeyCode::Unknown, // 129   SDL_SCANCODE_VOLUMEDOWN,
  (unsigned char)KeyCode::Unknown, // 130   SDL_SCANCODE_LOCKINGCAPSLOCK,
  (unsigned char)KeyCode::Unknown, // 131   SDL_SCANCODE_LOCKINGNUMLOCK,
  (unsigned char)KeyCode::Unknown, // 132   SDL_SCANCODE_LOCKINGSCROLLLOCK,
  (unsigned char)KeyCode::NumpadComma, // 133   SDL_SCANCODE_KP_COMMA,
  (unsigned char)KeyCode::Unknown, // 134   SDL_SCANCODE_KP_EQUALSAS400,
  (unsigned char)KeyCode::Unknown, // 135   SDL_SCANCODE_INTERNATIONAL1,
  (unsigned char)KeyCode::Unknown, // 136   SDL_SCANCODE_INTERNATIONAL2,
  (unsigned char)KeyCode::Unknown, // 137   SDL_SCANCODE_INTERNATIONAL3,
  (unsigned char)KeyCode::Unknown, // 138   SDL_SCANCODE_INTERNATIONAL4,
  (unsigned char)KeyCode::Unknown, // 139   SDL_SCANCODE_INTERNATIONAL5,
  (unsigned char)KeyCode::Unknown, // 140   SDL_SCANCODE_INTERNATIONAL6,
  (unsigned char)KeyCode::Unknown, // 141   SDL_SCANCODE_INTERNATIONAL7,
  (unsigned char)KeyCode::Unknown, // 142   SDL_SCANCODE_INTERNATIONAL8,
  (unsigned char)KeyCode::Unknown, // 143   SDL_SCANCODE_INTERNATIONAL9,
  (unsigned char)KeyCode::Unknown, // 144   SDL_SCANCODE_LANG1,
  (unsigned char)KeyCode::Unknown, // 145   SDL_SCANCODE_LANG2,
  (unsigned char)KeyCode::Unknown, // 146   SDL_SCANCODE_LANG3,
  (unsigned char)KeyCode::Unknown, // 147   SDL_SCANCODE_LANG4,
  (unsigned char)KeyCode::Unknown, // 148   SDL_SCANCODE_LANG5,
  (unsigned char)KeyCode::Unknown, // 149   SDL_SCANCODE_LANG6,
  (unsigned char)KeyCode::Unknown, // 150   SDL_SCANCODE_LANG7,
  (unsigned char)KeyCode::Unknown, // 151   SDL_SCANCODE_LANG8,
  (unsigned char)KeyCode::Unknown, // 152   SDL_SCANCODE_LANG9,
  (unsigned char)KeyCode::Unknown, // 153   SDL_SCANCODE_ALTERASE,
  (unsigned char)KeyCode::Unknown, // 154   SDL_SCANCODE_SYSREQ,
  (unsigned char)KeyCode::Unknown, // 155   SDL_SCANCODE_CANCEL,
  (unsigned char)KeyCode::Unknown, // 156   SDL_SCANCODE_CLEAR,
  (unsigned char)KeyCode::Unknown, // 157   SDL_SCANCODE_PRIOR,
  (unsigned char)KeyCode::Unknown, // 158   SDL_SCANCODE_RETURN2,
  (unsigned char)KeyCode::Unknown, // 159   SDL_SCANCODE_SEPARATOR,
  (unsigned char)KeyCode::Unknown, // 160   SDL_SCANCODE_OUT,
  (unsigned char)KeyCode::Unknown, // 161   SDL_SCANCODE_OPER,
  (unsigned char)KeyCode::Unknown, // 162   SDL_SCANCODE_CLEARAGAIN,
  (unsigned char)KeyCode::Unknown, // 163   SDL_SCANCODE_CRSEL,
  (unsigned char)KeyCode::Unknown, // 164   SDL_SCANCODE_EXSEL,
  (unsigned char)KeyCode::Unknown, // 165
  (unsigned char)KeyCode::Unknown, // 166
  (unsigned char)KeyCode::Unknown, // 167
  (unsigned char)KeyCode::Unknown, // 168
  (unsigned char)KeyCode::Unknown, // 169
  (unsigned char)KeyCode::Unknown, // 170
  (unsigned char)KeyCode::Unknown, // 171
  (unsigned char)KeyCode::Unknown, // 712
  (unsigned char)KeyCode::Unknown, // 173
  (unsigned char)KeyCode::Unknown, // 174
  (unsigned char)KeyCode::Unknown, // 175
  (unsigned char)KeyCode::Unknown, // 176   SDL_SCANCODE_KP_00,
  (unsigned char)KeyCode::Unknown, // 177   SDL_SCANCODE_KP_000,
  (unsigned char)KeyCode::Unknown, // 178   SDL_SCANCODE_THOUSANDSSEPARATOR,
  (unsigned char)KeyCode::Unknown, // 179   SDL_SCANCODE_DECIMALSEPARATOR,
  (unsigned char)KeyCode::Unknown, // 180   SDL_SCANCODE_CURRENCYUNIT,
  (unsigned char)KeyCode::Unknown, // 181   SDL_SCANCODE_CURRENCYSUBUNIT,
  (unsigned char)KeyCode::Unknown, // 182   SDL_SCANCODE_KP_LEFTPAREN,
  (unsigned char)KeyCode::Unknown, // 183   SDL_SCANCODE_KP_RIGHTPAREN,
  (unsigned char)KeyCode::Unknown, // 184   SDL_SCANCODE_KP_LEFTBRACE,
  (unsigned char)KeyCode::Unknown, // 185   SDL_SCANCODE_KP_RIGHTBRACE,
  (unsigned char)KeyCode::Unknown, // 186   SDL_SCANCODE_KP_TAB,
  (unsigned char)KeyCode::Unknown, // 187   SDL_SCANCODE_KP_BACKSPACE,
  (unsigned char)KeyCode::Unknown, // 188   SDL_SCANCODE_KP_A,
  (unsigned char)KeyCode::Unknown, // 189   SDL_SCANCODE_KP_B,
  (unsigned char)KeyCode::Unknown, // 190   SDL_SCANCODE_KP_C,
  (unsigned char)KeyCode::Unknown, // 191   SDL_SCANCODE_KP_D,
  (unsigned char)KeyCode::Unknown, // 192   SDL_SCANCODE_KP_E,
  (unsigned char)KeyCode::Unknown, // 193   SDL_SCANCODE_KP_F,
  (unsigned char)KeyCode::Unknown, // 194   SDL_SCANCODE_KP_XOR,
  (unsigned char)KeyCode::Unknown, // 195   SDL_SCANCODE_KP_POWER,
  (unsigned char)KeyCode::Unknown, // 196   SDL_SCANCODE_KP_PERCENT,
  (unsigned char)KeyCode::Unknown, // 197   SDL_SCANCODE_KP_LESS,
  (unsigned char)KeyCode::Unknown, // 198   SDL_SCANCODE_KP_GREATER,
  (unsigned char)KeyCode::Unknown, // 199   SDL_SCANCODE_KP_AMPERSAND,
  (unsigned char)KeyCode::Unknown, // 200   SDL_SCANCODE_KP_DBLAMPERSAND,
  (unsigned char)KeyCode::Unknown, // 201   SDL_SCANCODE_KP_VERTICALBAR,
  (unsigned char)KeyCode::Unknown, // 202   SDL_SCANCODE_KP_DBLVERTICALBAR,
  (unsigned char)KeyCode::Unknown, // 203   SDL_SCANCODE_KP_COLON,
  (unsigned char)KeyCode::Unknown, // 204   SDL_SCANCODE_KP_HASH,
  (unsigned char)KeyCode::Unknown, // 205   SDL_SCANCODE_KP_SPACE,
  (unsigned char)KeyCode::Unknown, // 206   SDL_SCANCODE_KP_AT,
  (unsigned char)KeyCode::Unknown, // 207   SDL_SCANCODE_KP_EXCLAM,
  (unsigned char)KeyCode::Unknown, // 208   SDL_SCANCODE_KP_MEMSTORE,
  (unsigned char)KeyCode::Unknown, // 209   SDL_SCANCODE_KP_MEMRECALL,
  (unsigned char)KeyCode::Unknown, // 210   SDL_SCANCODE_KP_MEMCLEAR,
  (unsigned char)KeyCode::Unknown, // 211   SDL_SCANCODE_KP_MEMADD,
  (unsigned char)KeyCode::Unknown, // 212   SDL_SCANCODE_KP_MEMSUBTRACT,
  (unsigned char)KeyCode::Unknown, // 213   SDL_SCANCODE_KP_MEMMULTIPLY,
  (unsigned char)KeyCode::Unknown, // 214   SDL_SCANCODE_KP_MEMDIVIDE,
  (unsigned char)KeyCode::Unknown, // 215   SDL_SCANCODE_KP_PLUSMINUS,
  (unsigned char)KeyCode::Unknown, // 216   SDL_SCANCODE_KP_CLEAR,
  (unsigned char)KeyCode::Unknown, // 217   SDL_SCANCODE_KP_CLEARENTRY,
  (unsigned char)KeyCode::Unknown, // 218   SDL_SCANCODE_KP_BINARY,
  (unsigned char)KeyCode::Unknown, // 219   SDL_SCANCODE_KP_OCTAL,
  (unsigned char)KeyCode::Unknown, // 220   SDL_SCANCODE_KP_DECIMAL,
  (unsigned char)KeyCode::Unknown, // 221   SDL_SCANCODE_KP_HEXADECIMAL,
  (unsigned char)KeyCode::Unknown, // 222
  (unsigned char)KeyCode::Unknown, // 223
  (unsigned char)KeyCode::LCtrl, // 224   SDL_SCANCODE_LCTRL,
  (unsigned char)KeyCode::LShift, // 225   SDL_SCANCODE_LSHIFT,
  (unsigned char)KeyCode::LAlt, // 226   SDL_SCANCODE_LALT,
  (unsigned char)KeyCode::LWin, // 227   SDL_SCANCODE_LGUI,
  (unsigned char)KeyCode::RCtrl, // 228   SDL_SCANCODE_RCTRL,
  (unsigned char)KeyCode::RShift, // 229   SDL_SCANCODE_RSHIFT,
  (unsigned char)KeyCode::RAlt, // 230   SDL_SCANCODE_RALT,
  (unsigned char)KeyCode::RWin, // 231   SDL_SCANCODE_RGUI,
  (unsigned char)KeyCode::Unknown, // 232
  (unsigned char)KeyCode::Unknown, // 233
  (unsigned char)KeyCode::Unknown, // 234
  (unsigned char)KeyCode::Unknown, // 235
  (unsigned char)KeyCode::Unknown, // 236
  (unsigned char)KeyCode::Unknown, // 237
  (unsigned char)KeyCode::Unknown, // 238
  (unsigned char)KeyCode::Unknown, // 239
  (unsigned char)KeyCode::Unknown, // 240
  (unsigned char)KeyCode::Unknown, // 241
  (unsigned char)KeyCode::Unknown, // 242
  (unsigned char)KeyCode::Unknown, // 243
  (unsigned char)KeyCode::Unknown, // 244
  (unsigned char)KeyCode::Unknown, // 245
  (unsigned char)KeyCode::Unknown, // 246
  (unsigned char)KeyCode::Unknown, // 247
  (unsigned char)KeyCode::Unknown, // 248
  (unsigned char)KeyCode::Unknown, // 249
  (unsigned char)KeyCode::Unknown, // 250
  (unsigned char)KeyCode::Unknown, // 251
  (unsigned char)KeyCode::Unknown, // 252
  (unsigned char)KeyCode::Unknown, // 253
  (unsigned char)KeyCode::Unknown, // 254
  (unsigned char)KeyCode::Unknown, // 255
  (unsigned char)KeyCode::Unknown, // 256
  (unsigned char)KeyCode::Unknown, // 257   SDL_SCANCODE_MODE,
  (unsigned char)KeyCode::Unknown, // 258   SDL_SCANCODE_AUDIONEXT,
  (unsigned char)KeyCode::Unknown, // 259   SDL_SCANCODE_AUDIOPREV,
  (unsigned char)KeyCode::Unknown, // 260   SDL_SCANCODE_AUDIOSTOP,
  (unsigned char)KeyCode::Unknown, // 261   SDL_SCANCODE_AUDIOPLAY,
  (unsigned char)KeyCode::Unknown, // 262   SDL_SCANCODE_AUDIOMUTE,
  (unsigned char)KeyCode::Unknown, // 263   SDL_SCANCODE_MEDIASELECT,
  (unsigned char)KeyCode::Unknown, // 264   SDL_SCANCODE_WWW,
  (unsigned char)KeyCode::Unknown, // 265   SDL_SCANCODE_MAIL,
  (unsigned char)KeyCode::Unknown, // 266   SDL_SCANCODE_CALCULATOR,
  (unsigned char)KeyCode::Unknown, // 267   SDL_SCANCODE_COMPUTER,
  (unsigned char)KeyCode::Unknown, // 268   SDL_SCANCODE_AC_SEARCH,
  (unsigned char)KeyCode::Unknown, // 269   SDL_SCANCODE_AC_HOME,
  (unsigned char)KeyCode::Unknown, // 270   SDL_SCANCODE_AC_BACK,
  (unsigned char)KeyCode::Unknown, // 271   SDL_SCANCODE_AC_FORWARD,
  (unsigned char)KeyCode::Unknown, // 272   SDL_SCANCODE_AC_STOP,
  (unsigned char)KeyCode::Unknown, // 273   SDL_SCANCODE_AC_REFRESH,
  (unsigned char)KeyCode::Unknown, // 274   SDL_SCANCODE_AC_BOOKMARKS,
  (unsigned char)KeyCode::Unknown, // 275   SDL_SCANCODE_BRIGHTNESSDOWN,
  (unsigned char)KeyCode::Unknown, // 276   SDL_SCANCODE_BRIGHTNESSUP,
  (unsigned char)KeyCode::Unknown, // 277   SDL_SCANCODE_DISPLAYSWITCH,
  (unsigned char)KeyCode::Unknown, // 278   SDL_SCANCODE_KBDILLUMTOGGLE,
  (unsigned char)KeyCode::Unknown, // 279   SDL_SCANCODE_KBDILLUMDOWN,
  (unsigned char)KeyCode::Unknown, // 280   SDL_SCANCODE_KBDILLUMUP,
  (unsigned char)KeyCode::Unknown, // 281   SDL_SCANCODE_EJECT,
  (unsigned char)KeyCode::Unknown  // 282   SDL_SCANCODE_SLEEP
};

// --------------------------------------------------------
// Author: Manu Evans, March 2015
void epInput_InitInternal()
{

}

// --------------------------------------------------------
// Author: Manu Evans, March 2015
void epInput_UpdateInternal()
{
  InputState &input = gInputState[gCurrentInputState];
  InputState &prev = gInputState[1 - gCurrentInputState];

  // poll keyboard
  const int MaxKeys = sizeof(sdlScanCodeToEPKey)/sizeof(sdlScanCodeToEPKey[0]);
  int numKeys;
  const Uint8 *state = SDL_GetKeyboardState(&numKeys);
  numKeys = numKeys < MaxKeys ? numKeys : MaxKeys;
  for(int i=0; i<numKeys; ++i)
  {
    if((KeyCode)sdlScanCodeToEPKey[i] == KeyCode::Unknown)
      continue;
    input.keys[0][sdlScanCodeToEPKey[i]] = (unsigned char)state[i];
  }

  // poll mouse
  int x, y;
  Uint32 buttons = SDL_GetMouseState(&x, &y);
  input.mouse[0][MouseControls::XAbsolute] = (float)x;
  input.mouse[0][MouseControls::YAbsolute] = (float)y;
  input.mouse[0][MouseControls::XDelta] = input.mouse[0][MouseControls::XAbsolute] - prev.mouse[0][MouseControls::XAbsolute];
  input.mouse[0][MouseControls::YDelta] = input.mouse[0][MouseControls::YAbsolute] - prev.mouse[0][MouseControls::YAbsolute];
  input.mouse[0][MouseControls::LeftButton] = (buttons & SDL_BUTTON_LMASK) ? 1.f : 0.f;
  input.mouse[0][MouseControls::RightButton] = (buttons & SDL_BUTTON_RMASK) ? 1.f : 0.f;
  input.mouse[0][MouseControls::MiddleButton] = (buttons & SDL_BUTTON_MMASK) ? 1.f : 0.f;
  input.mouse[0][MouseControls::Button4] = (buttons & SDL_BUTTON_X1MASK) ? 1.f : 0.f;
  input.mouse[0][MouseControls::Button5] = (buttons & SDL_BUTTON_X2MASK) ? 1.f : 0.f;

  // poll gamepads
  //...
}

#else
EPEMPTYFILE
#endif
