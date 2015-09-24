#include "hal/driver.h"

#if EPINPUT_DRIVER == EPDRIVER_SDL

#include "hal/input_internal.h"

#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_gamecontroller.h>

static unsigned char sdlScanCodeToUDKey[] =
{
  epKC_Unknown, // 0     SDL_SCANCODE_UNKNOWN,
  epKC_Unknown, // 1
  epKC_Unknown, // 2
  epKC_Unknown, // 3
  epKC_A, // 4     SDL_SCANCODE_A,
  epKC_B, // 5     SDL_SCANCODE_B,
  epKC_C, // 6     SDL_SCANCODE_C,
  epKC_D, // 7     SDL_SCANCODE_D,
  epKC_E, // 8     SDL_SCANCODE_E,
  epKC_F, // 9     SDL_SCANCODE_F,
  epKC_G, // 10    SDL_SCANCODE_G,
  epKC_H, // 11    SDL_SCANCODE_H,
  epKC_I, // 12    SDL_SCANCODE_I,
  epKC_J, // 13    SDL_SCANCODE_J,
  epKC_K, // 14    SDL_SCANCODE_K,
  epKC_L, // 15    SDL_SCANCODE_L,
  epKC_M, // 16    SDL_SCANCODE_M,
  epKC_N, // 17    SDL_SCANCODE_N,
  epKC_O, // 18    SDL_SCANCODE_O,
  epKC_P, // 19    SDL_SCANCODE_P,
  epKC_Q, // 20    SDL_SCANCODE_Q,
  epKC_R, // 21    SDL_SCANCODE_R,
  epKC_S, // 22    SDL_SCANCODE_S,
  epKC_T, // 23    SDL_SCANCODE_T,
  epKC_U, // 24    SDL_SCANCODE_U,
  epKC_V, // 25    SDL_SCANCODE_V,
  epKC_W, // 26    SDL_SCANCODE_W,
  epKC_X, // 27    SDL_SCANCODE_X,
  epKC_Y, // 28    SDL_SCANCODE_Y,
  epKC_Z, // 29    SDL_SCANCODE_Z,
  epKC_1, // 30    SDL_SCANCODE_1,
  epKC_2, // 31    SDL_SCANCODE_2,
  epKC_3, // 32    SDL_SCANCODE_3,
  epKC_4, // 33    SDL_SCANCODE_4,
  epKC_5, // 34    SDL_SCANCODE_5,
  epKC_6, // 35    SDL_SCANCODE_6,
  epKC_7, // 36    SDL_SCANCODE_7,
  epKC_8, // 37    SDL_SCANCODE_8,
  epKC_9, // 38    SDL_SCANCODE_9,
  epKC_0, // 39    SDL_SCANCODE_0,
  epKC_Enter, // 40    SDL_SCANCODE_RETURN,
  epKC_Escape, // 41    SDL_SCANCODE_ESCAPE,
  epKC_Backspace, // 42    SDL_SCANCODE_BACKSPACE,
  epKC_Tab, // 43    SDL_SCANCODE_TAB,
  epKC_Space, // 44    SDL_SCANCODE_SPACE,
  epKC_Hyphen, // 45    SDL_SCANCODE_MINUS,
  epKC_Equals, // 46    SDL_SCANCODE_EQUALS,
  epKC_LeftBracket, // 47    SDL_SCANCODE_LEFTBRACKET,
  epKC_RightBracket, // 48    SDL_SCANCODE_RIGHTBRACKET,
  epKC_BackSlash, // 49    SDL_SCANCODE_BACKSLASH,
  epKC_Unknown, // 50    SDL_SCANCODE_NONUSHASH,
  epKC_Semicolon, // 51    SDL_SCANCODE_SEMICOLON,
  epKC_Apostrophe, // 52    SDL_SCANCODE_APOSTROPHE,
  epKC_Grave, // 53    SDL_SCANCODE_GRAVE,
  epKC_Comma, // 54    SDL_SCANCODE_COMMA,
  epKC_Period, // 55    SDL_SCANCODE_PERIOD,
  epKC_ForwardSlash, // 56    SDL_SCANCODE_SLASH,
  epKC_CapsLock, // 57    SDL_SCANCODE_CAPSLOCK,
  epKC_F1, // 58    SDL_SCANCODE_F1,
  epKC_F2, // 59    SDL_SCANCODE_F2,
  epKC_F3, // 60    SDL_SCANCODE_F3,
  epKC_F4, // 61    SDL_SCANCODE_F4,
  epKC_F5, // 62    SDL_SCANCODE_F5,
  epKC_F6, // 63    SDL_SCANCODE_F6,
  epKC_F7, // 64    SDL_SCANCODE_F7,
  epKC_F8, // 65    SDL_SCANCODE_F8,
  epKC_F9, // 66    SDL_SCANCODE_F9,
  epKC_F10, // 67    SDL_SCANCODE_F10,
  epKC_F11, // 68    SDL_SCANCODE_F11,
  epKC_F12, // 69    SDL_SCANCODE_F12,
  epKC_PrintScreen, // 70    SDL_SCANCODE_PRINTSCREEN,
  epKC_ScrollLock, // 71    SDL_SCANCODE_SCROLLLOCK,
  epKC_Pause, // 72    SDL_SCANCODE_PAUSE,
  epKC_Insert, // 73    SDL_SCANCODE_INSERT,
  epKC_Home, // 74    SDL_SCANCODE_HOME,
  epKC_PageUp, // 75    SDL_SCANCODE_PAGEUP,
  epKC_Delete, // 76    SDL_SCANCODE_DELETE,
  epKC_End, // 77    SDL_SCANCODE_END,
  epKC_PageDown, // 78    SDL_SCANCODE_PAGEDOWN,
  epKC_Right, // 79    SDL_SCANCODE_RIGHT,
  epKC_Left, // 80    SDL_SCANCODE_LEFT,
  epKC_Down, // 81    SDL_SCANCODE_DOWN,
  epKC_Up, // 82    SDL_SCANCODE_UP,
  epKC_NumLock, // 83    SDL_SCANCODE_NUMLOCKCLEAR,
  epKC_NumpadDivide, // 84    SDL_SCANCODE_KP_DIVIDE,
  epKC_NumpadMultiply, // 85    SDL_SCANCODE_KP_MULTIPLY,
  epKC_NumpadMinus, // 86    SDL_SCANCODE_KP_MINUS,
  epKC_NumpadPlus, // 87    SDL_SCANCODE_KP_PLUS,
  epKC_NumpadEnter, // 88    SDL_SCANCODE_KP_ENTER,
  epKC_Numpad1, // 89    SDL_SCANCODE_KP_1,
  epKC_Numpad2, // 90    SDL_SCANCODE_KP_2,
  epKC_Numpad3, // 91    SDL_SCANCODE_KP_3,
  epKC_Numpad4, // 92    SDL_SCANCODE_KP_4,
  epKC_Numpad5, // 93    SDL_SCANCODE_KP_5,
  epKC_Numpad6, // 94    SDL_SCANCODE_KP_6,
  epKC_Numpad7, // 95    SDL_SCANCODE_KP_7,
  epKC_Numpad8, // 96    SDL_SCANCODE_KP_8,
  epKC_Numpad9, // 97    SDL_SCANCODE_KP_9,
  epKC_Numpad0, // 98    SDL_SCANCODE_KP_0,
  epKC_NumpadDecimal, // 99    SDL_SCANCODE_KP_PERIOD,
  epKC_Unknown, // 100   SDL_SCANCODE_NONUSBACKSLASH,
  epKC_Unknown, // 101   SDL_SCANCODE_APPLICATION,
  epKC_Unknown, // 102   SDL_SCANCODE_POWER,
  epKC_NumpadEquals, // 103   SDL_SCANCODE_KP_EQUALS,
  epKC_Unknown, // 104   SDL_SCANCODE_F13,
  epKC_Unknown, // 105   SDL_SCANCODE_F14,
  epKC_Unknown, // 106   SDL_SCANCODE_F15,
  epKC_Unknown, // 107   SDL_SCANCODE_F16,
  epKC_Unknown, // 108   SDL_SCANCODE_F17,
  epKC_Unknown, // 109   SDL_SCANCODE_F18,
  epKC_Unknown, // 110   SDL_SCANCODE_F19,
  epKC_Unknown, // 111   SDL_SCANCODE_F20,
  epKC_Unknown, // 112   SDL_SCANCODE_F21,
  epKC_Unknown, // 113   SDL_SCANCODE_F22,
  epKC_Unknown, // 114   SDL_SCANCODE_F23,
  epKC_Unknown, // 115   SDL_SCANCODE_F24,
  epKC_Unknown, // 116   SDL_SCANCODE_EXECUTE,
  epKC_Unknown, // 117   SDL_SCANCODE_HELP,
  epKC_Menu, // 118   SDL_SCANCODE_MENU,
  epKC_Unknown, // 119   SDL_SCANCODE_SELECT,
  epKC_Unknown, // 120   SDL_SCANCODE_STOP,
  epKC_Unknown, // 121   SDL_SCANCODE_AGAIN,
  epKC_Unknown, // 122   SDL_SCANCODE_UNDO,
  epKC_Unknown, // 123   SDL_SCANCODE_CUT,
  epKC_Unknown, // 124   SDL_SCANCODE_COPY,
  epKC_Unknown, // 125   SDL_SCANCODE_PASTE,
  epKC_Unknown, // 126   SDL_SCANCODE_FIND,
  epKC_Unknown, // 127   SDL_SCANCODE_MUTE,
  epKC_Unknown, // 128   SDL_SCANCODE_VOLUMEUP,
  epKC_Unknown, // 129   SDL_SCANCODE_VOLUMEDOWN,
  epKC_Unknown, // 130   SDL_SCANCODE_LOCKINGCAPSLOCK,
  epKC_Unknown, // 131   SDL_SCANCODE_LOCKINGNUMLOCK,
  epKC_Unknown, // 132   SDL_SCANCODE_LOCKINGSCROLLLOCK,
  epKC_NumpadComma, // 133   SDL_SCANCODE_KP_COMMA,
  epKC_Unknown, // 134   SDL_SCANCODE_KP_EQUALSAS400,
  epKC_Unknown, // 135   SDL_SCANCODE_INTERNATIONAL1,
  epKC_Unknown, // 136   SDL_SCANCODE_INTERNATIONAL2,
  epKC_Unknown, // 137   SDL_SCANCODE_INTERNATIONAL3,
  epKC_Unknown, // 138   SDL_SCANCODE_INTERNATIONAL4,
  epKC_Unknown, // 139   SDL_SCANCODE_INTERNATIONAL5,
  epKC_Unknown, // 140   SDL_SCANCODE_INTERNATIONAL6,
  epKC_Unknown, // 141   SDL_SCANCODE_INTERNATIONAL7,
  epKC_Unknown, // 142   SDL_SCANCODE_INTERNATIONAL8,
  epKC_Unknown, // 143   SDL_SCANCODE_INTERNATIONAL9,
  epKC_Unknown, // 144   SDL_SCANCODE_LANG1,
  epKC_Unknown, // 145   SDL_SCANCODE_LANG2,
  epKC_Unknown, // 146   SDL_SCANCODE_LANG3,
  epKC_Unknown, // 147   SDL_SCANCODE_LANG4,
  epKC_Unknown, // 148   SDL_SCANCODE_LANG5,
  epKC_Unknown, // 149   SDL_SCANCODE_LANG6,
  epKC_Unknown, // 150   SDL_SCANCODE_LANG7,
  epKC_Unknown, // 151   SDL_SCANCODE_LANG8,
  epKC_Unknown, // 152   SDL_SCANCODE_LANG9,
  epKC_Unknown, // 153   SDL_SCANCODE_ALTERASE,
  epKC_Unknown, // 154   SDL_SCANCODE_SYSREQ,
  epKC_Unknown, // 155   SDL_SCANCODE_CANCEL,
  epKC_Unknown, // 156   SDL_SCANCODE_CLEAR,
  epKC_Unknown, // 157   SDL_SCANCODE_PRIOR,
  epKC_Unknown, // 158   SDL_SCANCODE_RETURN2,
  epKC_Unknown, // 159   SDL_SCANCODE_SEPARATOR,
  epKC_Unknown, // 160   SDL_SCANCODE_OUT,
  epKC_Unknown, // 161   SDL_SCANCODE_OPER,
  epKC_Unknown, // 162   SDL_SCANCODE_CLEARAGAIN,
  epKC_Unknown, // 163   SDL_SCANCODE_CRSEL,
  epKC_Unknown, // 164   SDL_SCANCODE_EXSEL,
  epKC_Unknown, // 165
  epKC_Unknown, // 166
  epKC_Unknown, // 167
  epKC_Unknown, // 168
  epKC_Unknown, // 169
  epKC_Unknown, // 170
  epKC_Unknown, // 171
  epKC_Unknown, // 712
  epKC_Unknown, // 173
  epKC_Unknown, // 174
  epKC_Unknown, // 175
  epKC_Unknown, // 176   SDL_SCANCODE_KP_00,
  epKC_Unknown, // 177   SDL_SCANCODE_KP_000,
  epKC_Unknown, // 178   SDL_SCANCODE_THOUSANDSSEPARATOR,
  epKC_Unknown, // 179   SDL_SCANCODE_DECIMALSEPARATOR,
  epKC_Unknown, // 180   SDL_SCANCODE_CURRENCYUNIT,
  epKC_Unknown, // 181   SDL_SCANCODE_CURRENCYSUBUNIT,
  epKC_Unknown, // 182   SDL_SCANCODE_KP_LEFTPAREN,
  epKC_Unknown, // 183   SDL_SCANCODE_KP_RIGHTPAREN,
  epKC_Unknown, // 184   SDL_SCANCODE_KP_LEFTBRACE,
  epKC_Unknown, // 185   SDL_SCANCODE_KP_RIGHTBRACE,
  epKC_Unknown, // 186   SDL_SCANCODE_KP_TAB,
  epKC_Unknown, // 187   SDL_SCANCODE_KP_BACKSPACE,
  epKC_Unknown, // 188   SDL_SCANCODE_KP_A,
  epKC_Unknown, // 189   SDL_SCANCODE_KP_B,
  epKC_Unknown, // 190   SDL_SCANCODE_KP_C,
  epKC_Unknown, // 191   SDL_SCANCODE_KP_D,
  epKC_Unknown, // 192   SDL_SCANCODE_KP_E,
  epKC_Unknown, // 193   SDL_SCANCODE_KP_F,
  epKC_Unknown, // 194   SDL_SCANCODE_KP_XOR,
  epKC_Unknown, // 195   SDL_SCANCODE_KP_POWER,
  epKC_Unknown, // 196   SDL_SCANCODE_KP_PERCENT,
  epKC_Unknown, // 197   SDL_SCANCODE_KP_LESS,
  epKC_Unknown, // 198   SDL_SCANCODE_KP_GREATER,
  epKC_Unknown, // 199   SDL_SCANCODE_KP_AMPERSAND,
  epKC_Unknown, // 200   SDL_SCANCODE_KP_DBLAMPERSAND,
  epKC_Unknown, // 201   SDL_SCANCODE_KP_VERTICALBAR,
  epKC_Unknown, // 202   SDL_SCANCODE_KP_DBLVERTICALBAR,
  epKC_Unknown, // 203   SDL_SCANCODE_KP_COLON,
  epKC_Unknown, // 204   SDL_SCANCODE_KP_HASH,
  epKC_Unknown, // 205   SDL_SCANCODE_KP_SPACE,
  epKC_Unknown, // 206   SDL_SCANCODE_KP_AT,
  epKC_Unknown, // 207   SDL_SCANCODE_KP_EXCLAM,
  epKC_Unknown, // 208   SDL_SCANCODE_KP_MEMSTORE,
  epKC_Unknown, // 209   SDL_SCANCODE_KP_MEMRECALL,
  epKC_Unknown, // 210   SDL_SCANCODE_KP_MEMCLEAR,
  epKC_Unknown, // 211   SDL_SCANCODE_KP_MEMADD,
  epKC_Unknown, // 212   SDL_SCANCODE_KP_MEMSUBTRACT,
  epKC_Unknown, // 213   SDL_SCANCODE_KP_MEMMULTIPLY,
  epKC_Unknown, // 214   SDL_SCANCODE_KP_MEMDIVIDE,
  epKC_Unknown, // 215   SDL_SCANCODE_KP_PLUSMINUS,
  epKC_Unknown, // 216   SDL_SCANCODE_KP_CLEAR,
  epKC_Unknown, // 217   SDL_SCANCODE_KP_CLEARENTRY,
  epKC_Unknown, // 218   SDL_SCANCODE_KP_BINARY,
  epKC_Unknown, // 219   SDL_SCANCODE_KP_OCTAL,
  epKC_Unknown, // 220   SDL_SCANCODE_KP_DECIMAL,
  epKC_Unknown, // 221   SDL_SCANCODE_KP_HEXADECIMAL,
  epKC_Unknown, // 222
  epKC_Unknown, // 223
  epKC_LCtrl, // 224   SDL_SCANCODE_LCTRL,
  epKC_LShift, // 225   SDL_SCANCODE_LSHIFT,
  epKC_LAlt, // 226   SDL_SCANCODE_LALT,
  epKC_LWin, // 227   SDL_SCANCODE_LGUI,
  epKC_RCtrl, // 228   SDL_SCANCODE_RCTRL,
  epKC_RShift, // 229   SDL_SCANCODE_RSHIFT,
  epKC_RAlt, // 230   SDL_SCANCODE_RALT,
  epKC_RWin, // 231   SDL_SCANCODE_RGUI,
  epKC_Unknown, // 232
  epKC_Unknown, // 233
  epKC_Unknown, // 234
  epKC_Unknown, // 235
  epKC_Unknown, // 236
  epKC_Unknown, // 237
  epKC_Unknown, // 238
  epKC_Unknown, // 239
  epKC_Unknown, // 240
  epKC_Unknown, // 241
  epKC_Unknown, // 242
  epKC_Unknown, // 243
  epKC_Unknown, // 244
  epKC_Unknown, // 245
  epKC_Unknown, // 246
  epKC_Unknown, // 247
  epKC_Unknown, // 248
  epKC_Unknown, // 249
  epKC_Unknown, // 250
  epKC_Unknown, // 251
  epKC_Unknown, // 252
  epKC_Unknown, // 253
  epKC_Unknown, // 254
  epKC_Unknown, // 255
  epKC_Unknown, // 256
  epKC_Unknown, // 257   SDL_SCANCODE_MODE,
  epKC_Unknown, // 258   SDL_SCANCODE_AUDIONEXT,
  epKC_Unknown, // 259   SDL_SCANCODE_AUDIOPREV,
  epKC_Unknown, // 260   SDL_SCANCODE_AUDIOSTOP,
  epKC_Unknown, // 261   SDL_SCANCODE_AUDIOPLAY,
  epKC_Unknown, // 262   SDL_SCANCODE_AUDIOMUTE,
  epKC_Unknown, // 263   SDL_SCANCODE_MEDIASELECT,
  epKC_Unknown, // 264   SDL_SCANCODE_WWW,
  epKC_Unknown, // 265   SDL_SCANCODE_MAIL,
  epKC_Unknown, // 266   SDL_SCANCODE_CALCULATOR,
  epKC_Unknown, // 267   SDL_SCANCODE_COMPUTER,
  epKC_Unknown, // 268   SDL_SCANCODE_AC_SEARCH,
  epKC_Unknown, // 269   SDL_SCANCODE_AC_HOME,
  epKC_Unknown, // 270   SDL_SCANCODE_AC_BACK,
  epKC_Unknown, // 271   SDL_SCANCODE_AC_FORWARD,
  epKC_Unknown, // 272   SDL_SCANCODE_AC_STOP,
  epKC_Unknown, // 273   SDL_SCANCODE_AC_REFRESH,
  epKC_Unknown, // 274   SDL_SCANCODE_AC_BOOKMARKS,
  epKC_Unknown, // 275   SDL_SCANCODE_BRIGHTNESSDOWN,
  epKC_Unknown, // 276   SDL_SCANCODE_BRIGHTNESSUP,
  epKC_Unknown, // 277   SDL_SCANCODE_DISPLAYSWITCH,
  epKC_Unknown, // 278   SDL_SCANCODE_KBDILLUMTOGGLE,
  epKC_Unknown, // 279   SDL_SCANCODE_KBDILLUMDOWN,
  epKC_Unknown, // 280   SDL_SCANCODE_KBDILLUMUP,
  epKC_Unknown, // 281   SDL_SCANCODE_EJECT,
  epKC_Unknown  // 282   SDL_SCANCODE_SLEEP
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
  const int MaxKeys = sizeof(sdlScanCodeToUDKey)/sizeof(sdlScanCodeToUDKey[0]);
  int numKeys;
  const Uint8 *state = SDL_GetKeyboardState(&numKeys);
  numKeys = numKeys < MaxKeys ? numKeys : MaxKeys;
  for(int i=0; i<numKeys; ++i)
  {
    if(sdlScanCodeToUDKey[i] == epKC_Unknown)
      continue;
    input.keys[0][sdlScanCodeToUDKey[i]] = (unsigned char)state[i];
  }

  // poll mouse
  int x, y;
  Uint32 buttons = SDL_GetMouseState(&x, &y);
  input.mouse[0][epMC_XAbsolute] = (float)x;
  input.mouse[0][epMC_YAbsolute] = (float)y;
  input.mouse[0][epMC_XDelta] = input.mouse[0][epMC_XAbsolute] - prev.mouse[0][epMC_XAbsolute];
  input.mouse[0][epMC_YDelta] = input.mouse[0][epMC_YAbsolute] - prev.mouse[0][epMC_YAbsolute];
  input.mouse[0][epMC_LeftButton] = (buttons & SDL_BUTTON_LMASK) ? 1.f : 0.f;
  input.mouse[0][epMC_RightButton] = (buttons & SDL_BUTTON_RMASK) ? 1.f : 0.f;
  input.mouse[0][epMC_MiddleButton] = (buttons & SDL_BUTTON_MMASK) ? 1.f : 0.f;
  input.mouse[0][epMC_Button4] = (buttons & SDL_BUTTON_X1MASK) ? 1.f : 0.f;
  input.mouse[0][epMC_Button5] = (buttons & SDL_BUTTON_X2MASK) ? 1.f : 0.f;

  // poll gamepads
  //...
}

#endif
