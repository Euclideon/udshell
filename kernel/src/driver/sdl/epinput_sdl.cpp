#include "hal/driver.h"

#if UDINPUT_DRIVER == UDDRIVER_SDL

#include "hal/input_internal.h"

#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_gamecontroller.h>

static unsigned char sdlScanCodeToUDKey[] =
{
  udKC_Unknown, // 0     SDL_SCANCODE_UNKNOWN,
  udKC_Unknown, // 1
  udKC_Unknown, // 2
  udKC_Unknown, // 3
  udKC_A, // 4     SDL_SCANCODE_A,
  udKC_B, // 5     SDL_SCANCODE_B,
  udKC_C, // 6     SDL_SCANCODE_C,
  udKC_D, // 7     SDL_SCANCODE_D,
  udKC_E, // 8     SDL_SCANCODE_E,
  udKC_F, // 9     SDL_SCANCODE_F,
  udKC_G, // 10    SDL_SCANCODE_G,
  udKC_H, // 11    SDL_SCANCODE_H,
  udKC_I, // 12    SDL_SCANCODE_I,
  udKC_J, // 13    SDL_SCANCODE_J,
  udKC_K, // 14    SDL_SCANCODE_K,
  udKC_L, // 15    SDL_SCANCODE_L,
  udKC_M, // 16    SDL_SCANCODE_M,
  udKC_N, // 17    SDL_SCANCODE_N,
  udKC_O, // 18    SDL_SCANCODE_O,
  udKC_P, // 19    SDL_SCANCODE_P,
  udKC_Q, // 20    SDL_SCANCODE_Q,
  udKC_R, // 21    SDL_SCANCODE_R,
  udKC_S, // 22    SDL_SCANCODE_S,
  udKC_T, // 23    SDL_SCANCODE_T,
  udKC_U, // 24    SDL_SCANCODE_U,
  udKC_V, // 25    SDL_SCANCODE_V,
  udKC_W, // 26    SDL_SCANCODE_W,
  udKC_X, // 27    SDL_SCANCODE_X,
  udKC_Y, // 28    SDL_SCANCODE_Y,
  udKC_Z, // 29    SDL_SCANCODE_Z,
  udKC_1, // 30    SDL_SCANCODE_1,
  udKC_2, // 31    SDL_SCANCODE_2,
  udKC_3, // 32    SDL_SCANCODE_3,
  udKC_4, // 33    SDL_SCANCODE_4,
  udKC_5, // 34    SDL_SCANCODE_5,
  udKC_6, // 35    SDL_SCANCODE_6,
  udKC_7, // 36    SDL_SCANCODE_7,
  udKC_8, // 37    SDL_SCANCODE_8,
  udKC_9, // 38    SDL_SCANCODE_9,
  udKC_0, // 39    SDL_SCANCODE_0,
  udKC_Enter, // 40    SDL_SCANCODE_RETURN,
  udKC_Escape, // 41    SDL_SCANCODE_ESCAPE,
  udKC_Backspace, // 42    SDL_SCANCODE_BACKSPACE,
  udKC_Tab, // 43    SDL_SCANCODE_TAB,
  udKC_Space, // 44    SDL_SCANCODE_SPACE,
  udKC_Hyphen, // 45    SDL_SCANCODE_MINUS,
  udKC_Equals, // 46    SDL_SCANCODE_EQUALS,
  udKC_LeftBracket, // 47    SDL_SCANCODE_LEFTBRACKET,
  udKC_RightBracket, // 48    SDL_SCANCODE_RIGHTBRACKET,
  udKC_BackSlash, // 49    SDL_SCANCODE_BACKSLASH,
  udKC_Unknown, // 50    SDL_SCANCODE_NONUSHASH,
  udKC_Semicolon, // 51    SDL_SCANCODE_SEMICOLON,
  udKC_Apostrophe, // 52    SDL_SCANCODE_APOSTROPHE,
  udKC_Grave, // 53    SDL_SCANCODE_GRAVE,
  udKC_Comma, // 54    SDL_SCANCODE_COMMA,
  udKC_Period, // 55    SDL_SCANCODE_PERIOD,
  udKC_ForwardSlash, // 56    SDL_SCANCODE_SLASH,
  udKC_CapsLock, // 57    SDL_SCANCODE_CAPSLOCK,
  udKC_F1, // 58    SDL_SCANCODE_F1,
  udKC_F2, // 59    SDL_SCANCODE_F2,
  udKC_F3, // 60    SDL_SCANCODE_F3,
  udKC_F4, // 61    SDL_SCANCODE_F4,
  udKC_F5, // 62    SDL_SCANCODE_F5,
  udKC_F6, // 63    SDL_SCANCODE_F6,
  udKC_F7, // 64    SDL_SCANCODE_F7,
  udKC_F8, // 65    SDL_SCANCODE_F8,
  udKC_F9, // 66    SDL_SCANCODE_F9,
  udKC_F10, // 67    SDL_SCANCODE_F10,
  udKC_F11, // 68    SDL_SCANCODE_F11,
  udKC_F12, // 69    SDL_SCANCODE_F12,
  udKC_PrintScreen, // 70    SDL_SCANCODE_PRINTSCREEN,
  udKC_ScrollLock, // 71    SDL_SCANCODE_SCROLLLOCK,
  udKC_Pause, // 72    SDL_SCANCODE_PAUSE,
  udKC_Insert, // 73    SDL_SCANCODE_INSERT,
  udKC_Home, // 74    SDL_SCANCODE_HOME,
  udKC_PageUp, // 75    SDL_SCANCODE_PAGEUP,
  udKC_Delete, // 76    SDL_SCANCODE_DELETE,
  udKC_End, // 77    SDL_SCANCODE_END,
  udKC_PageDown, // 78    SDL_SCANCODE_PAGEDOWN,
  udKC_Right, // 79    SDL_SCANCODE_RIGHT,
  udKC_Left, // 80    SDL_SCANCODE_LEFT,
  udKC_Down, // 81    SDL_SCANCODE_DOWN,
  udKC_Up, // 82    SDL_SCANCODE_UP,
  udKC_NumLock, // 83    SDL_SCANCODE_NUMLOCKCLEAR,
  udKC_NumpadDivide, // 84    SDL_SCANCODE_KP_DIVIDE,
  udKC_NumpadMultiply, // 85    SDL_SCANCODE_KP_MULTIPLY,
  udKC_NumpadMinus, // 86    SDL_SCANCODE_KP_MINUS,
  udKC_NumpadPlus, // 87    SDL_SCANCODE_KP_PLUS,
  udKC_NumpadEnter, // 88    SDL_SCANCODE_KP_ENTER,
  udKC_Numpad1, // 89    SDL_SCANCODE_KP_1,
  udKC_Numpad2, // 90    SDL_SCANCODE_KP_2,
  udKC_Numpad3, // 91    SDL_SCANCODE_KP_3,
  udKC_Numpad4, // 92    SDL_SCANCODE_KP_4,
  udKC_Numpad5, // 93    SDL_SCANCODE_KP_5,
  udKC_Numpad6, // 94    SDL_SCANCODE_KP_6,
  udKC_Numpad7, // 95    SDL_SCANCODE_KP_7,
  udKC_Numpad8, // 96    SDL_SCANCODE_KP_8,
  udKC_Numpad9, // 97    SDL_SCANCODE_KP_9,
  udKC_Numpad0, // 98    SDL_SCANCODE_KP_0,
  udKC_NumpadDecimal, // 99    SDL_SCANCODE_KP_PERIOD,
  udKC_Unknown, // 100   SDL_SCANCODE_NONUSBACKSLASH,
  udKC_Unknown, // 101   SDL_SCANCODE_APPLICATION,
  udKC_Unknown, // 102   SDL_SCANCODE_POWER,
  udKC_NumpadEquals, // 103   SDL_SCANCODE_KP_EQUALS,
  udKC_Unknown, // 104   SDL_SCANCODE_F13,
  udKC_Unknown, // 105   SDL_SCANCODE_F14,
  udKC_Unknown, // 106   SDL_SCANCODE_F15,
  udKC_Unknown, // 107   SDL_SCANCODE_F16,
  udKC_Unknown, // 108   SDL_SCANCODE_F17,
  udKC_Unknown, // 109   SDL_SCANCODE_F18,
  udKC_Unknown, // 110   SDL_SCANCODE_F19,
  udKC_Unknown, // 111   SDL_SCANCODE_F20,
  udKC_Unknown, // 112   SDL_SCANCODE_F21,
  udKC_Unknown, // 113   SDL_SCANCODE_F22,
  udKC_Unknown, // 114   SDL_SCANCODE_F23,
  udKC_Unknown, // 115   SDL_SCANCODE_F24,
  udKC_Unknown, // 116   SDL_SCANCODE_EXECUTE,
  udKC_Unknown, // 117   SDL_SCANCODE_HELP,
  udKC_Menu, // 118   SDL_SCANCODE_MENU,
  udKC_Unknown, // 119   SDL_SCANCODE_SELECT,
  udKC_Unknown, // 120   SDL_SCANCODE_STOP,
  udKC_Unknown, // 121   SDL_SCANCODE_AGAIN,
  udKC_Unknown, // 122   SDL_SCANCODE_UNDO,
  udKC_Unknown, // 123   SDL_SCANCODE_CUT,
  udKC_Unknown, // 124   SDL_SCANCODE_COPY,
  udKC_Unknown, // 125   SDL_SCANCODE_PASTE,
  udKC_Unknown, // 126   SDL_SCANCODE_FIND,
  udKC_Unknown, // 127   SDL_SCANCODE_MUTE,
  udKC_Unknown, // 128   SDL_SCANCODE_VOLUMEUP,
  udKC_Unknown, // 129   SDL_SCANCODE_VOLUMEDOWN,
  udKC_Unknown, // 130   SDL_SCANCODE_LOCKINGCAPSLOCK,
  udKC_Unknown, // 131   SDL_SCANCODE_LOCKINGNUMLOCK,
  udKC_Unknown, // 132   SDL_SCANCODE_LOCKINGSCROLLLOCK,
  udKC_NumpadComma, // 133   SDL_SCANCODE_KP_COMMA,
  udKC_Unknown, // 134   SDL_SCANCODE_KP_EQUALSAS400,
  udKC_Unknown, // 135   SDL_SCANCODE_INTERNATIONAL1,
  udKC_Unknown, // 136   SDL_SCANCODE_INTERNATIONAL2,
  udKC_Unknown, // 137   SDL_SCANCODE_INTERNATIONAL3,
  udKC_Unknown, // 138   SDL_SCANCODE_INTERNATIONAL4,
  udKC_Unknown, // 139   SDL_SCANCODE_INTERNATIONAL5,
  udKC_Unknown, // 140   SDL_SCANCODE_INTERNATIONAL6,
  udKC_Unknown, // 141   SDL_SCANCODE_INTERNATIONAL7,
  udKC_Unknown, // 142   SDL_SCANCODE_INTERNATIONAL8,
  udKC_Unknown, // 143   SDL_SCANCODE_INTERNATIONAL9,
  udKC_Unknown, // 144   SDL_SCANCODE_LANG1,
  udKC_Unknown, // 145   SDL_SCANCODE_LANG2,
  udKC_Unknown, // 146   SDL_SCANCODE_LANG3,
  udKC_Unknown, // 147   SDL_SCANCODE_LANG4,
  udKC_Unknown, // 148   SDL_SCANCODE_LANG5,
  udKC_Unknown, // 149   SDL_SCANCODE_LANG6,
  udKC_Unknown, // 150   SDL_SCANCODE_LANG7,
  udKC_Unknown, // 151   SDL_SCANCODE_LANG8,
  udKC_Unknown, // 152   SDL_SCANCODE_LANG9,
  udKC_Unknown, // 153   SDL_SCANCODE_ALTERASE,
  udKC_Unknown, // 154   SDL_SCANCODE_SYSREQ,
  udKC_Unknown, // 155   SDL_SCANCODE_CANCEL,
  udKC_Unknown, // 156   SDL_SCANCODE_CLEAR,
  udKC_Unknown, // 157   SDL_SCANCODE_PRIOR,
  udKC_Unknown, // 158   SDL_SCANCODE_RETURN2,
  udKC_Unknown, // 159   SDL_SCANCODE_SEPARATOR,
  udKC_Unknown, // 160   SDL_SCANCODE_OUT,
  udKC_Unknown, // 161   SDL_SCANCODE_OPER,
  udKC_Unknown, // 162   SDL_SCANCODE_CLEARAGAIN,
  udKC_Unknown, // 163   SDL_SCANCODE_CRSEL,
  udKC_Unknown, // 164   SDL_SCANCODE_EXSEL,
  udKC_Unknown, // 165
  udKC_Unknown, // 166
  udKC_Unknown, // 167
  udKC_Unknown, // 168
  udKC_Unknown, // 169
  udKC_Unknown, // 170
  udKC_Unknown, // 171
  udKC_Unknown, // 712
  udKC_Unknown, // 173
  udKC_Unknown, // 174
  udKC_Unknown, // 175
  udKC_Unknown, // 176   SDL_SCANCODE_KP_00,
  udKC_Unknown, // 177   SDL_SCANCODE_KP_000,
  udKC_Unknown, // 178   SDL_SCANCODE_THOUSANDSSEPARATOR,
  udKC_Unknown, // 179   SDL_SCANCODE_DECIMALSEPARATOR,
  udKC_Unknown, // 180   SDL_SCANCODE_CURRENCYUNIT,
  udKC_Unknown, // 181   SDL_SCANCODE_CURRENCYSUBUNIT,
  udKC_Unknown, // 182   SDL_SCANCODE_KP_LEFTPAREN,
  udKC_Unknown, // 183   SDL_SCANCODE_KP_RIGHTPAREN,
  udKC_Unknown, // 184   SDL_SCANCODE_KP_LEFTBRACE,
  udKC_Unknown, // 185   SDL_SCANCODE_KP_RIGHTBRACE,
  udKC_Unknown, // 186   SDL_SCANCODE_KP_TAB,
  udKC_Unknown, // 187   SDL_SCANCODE_KP_BACKSPACE,
  udKC_Unknown, // 188   SDL_SCANCODE_KP_A,
  udKC_Unknown, // 189   SDL_SCANCODE_KP_B,
  udKC_Unknown, // 190   SDL_SCANCODE_KP_C,
  udKC_Unknown, // 191   SDL_SCANCODE_KP_D,
  udKC_Unknown, // 192   SDL_SCANCODE_KP_E,
  udKC_Unknown, // 193   SDL_SCANCODE_KP_F,
  udKC_Unknown, // 194   SDL_SCANCODE_KP_XOR,
  udKC_Unknown, // 195   SDL_SCANCODE_KP_POWER,
  udKC_Unknown, // 196   SDL_SCANCODE_KP_PERCENT,
  udKC_Unknown, // 197   SDL_SCANCODE_KP_LESS,
  udKC_Unknown, // 198   SDL_SCANCODE_KP_GREATER,
  udKC_Unknown, // 199   SDL_SCANCODE_KP_AMPERSAND,
  udKC_Unknown, // 200   SDL_SCANCODE_KP_DBLAMPERSAND,
  udKC_Unknown, // 201   SDL_SCANCODE_KP_VERTICALBAR,
  udKC_Unknown, // 202   SDL_SCANCODE_KP_DBLVERTICALBAR,
  udKC_Unknown, // 203   SDL_SCANCODE_KP_COLON,
  udKC_Unknown, // 204   SDL_SCANCODE_KP_HASH,
  udKC_Unknown, // 205   SDL_SCANCODE_KP_SPACE,
  udKC_Unknown, // 206   SDL_SCANCODE_KP_AT,
  udKC_Unknown, // 207   SDL_SCANCODE_KP_EXCLAM,
  udKC_Unknown, // 208   SDL_SCANCODE_KP_MEMSTORE,
  udKC_Unknown, // 209   SDL_SCANCODE_KP_MEMRECALL,
  udKC_Unknown, // 210   SDL_SCANCODE_KP_MEMCLEAR,
  udKC_Unknown, // 211   SDL_SCANCODE_KP_MEMADD,
  udKC_Unknown, // 212   SDL_SCANCODE_KP_MEMSUBTRACT,
  udKC_Unknown, // 213   SDL_SCANCODE_KP_MEMMULTIPLY,
  udKC_Unknown, // 214   SDL_SCANCODE_KP_MEMDIVIDE,
  udKC_Unknown, // 215   SDL_SCANCODE_KP_PLUSMINUS,
  udKC_Unknown, // 216   SDL_SCANCODE_KP_CLEAR,
  udKC_Unknown, // 217   SDL_SCANCODE_KP_CLEARENTRY,
  udKC_Unknown, // 218   SDL_SCANCODE_KP_BINARY,
  udKC_Unknown, // 219   SDL_SCANCODE_KP_OCTAL,
  udKC_Unknown, // 220   SDL_SCANCODE_KP_DECIMAL,
  udKC_Unknown, // 221   SDL_SCANCODE_KP_HEXADECIMAL,
  udKC_Unknown, // 222
  udKC_Unknown, // 223
  udKC_LCtrl, // 224   SDL_SCANCODE_LCTRL,
  udKC_LShift, // 225   SDL_SCANCODE_LSHIFT,
  udKC_LAlt, // 226   SDL_SCANCODE_LALT,
  udKC_LWin, // 227   SDL_SCANCODE_LGUI,
  udKC_RCtrl, // 228   SDL_SCANCODE_RCTRL,
  udKC_RShift, // 229   SDL_SCANCODE_RSHIFT,
  udKC_RAlt, // 230   SDL_SCANCODE_RALT,
  udKC_RWin, // 231   SDL_SCANCODE_RGUI,
  udKC_Unknown, // 232
  udKC_Unknown, // 233
  udKC_Unknown, // 234
  udKC_Unknown, // 235
  udKC_Unknown, // 236
  udKC_Unknown, // 237
  udKC_Unknown, // 238
  udKC_Unknown, // 239
  udKC_Unknown, // 240
  udKC_Unknown, // 241
  udKC_Unknown, // 242
  udKC_Unknown, // 243
  udKC_Unknown, // 244
  udKC_Unknown, // 245
  udKC_Unknown, // 246
  udKC_Unknown, // 247
  udKC_Unknown, // 248
  udKC_Unknown, // 249
  udKC_Unknown, // 250
  udKC_Unknown, // 251
  udKC_Unknown, // 252
  udKC_Unknown, // 253
  udKC_Unknown, // 254
  udKC_Unknown, // 255
  udKC_Unknown, // 256
  udKC_Unknown, // 257   SDL_SCANCODE_MODE,
  udKC_Unknown, // 258   SDL_SCANCODE_AUDIONEXT,
  udKC_Unknown, // 259   SDL_SCANCODE_AUDIOPREV,
  udKC_Unknown, // 260   SDL_SCANCODE_AUDIOSTOP,
  udKC_Unknown, // 261   SDL_SCANCODE_AUDIOPLAY,
  udKC_Unknown, // 262   SDL_SCANCODE_AUDIOMUTE,
  udKC_Unknown, // 263   SDL_SCANCODE_MEDIASELECT,
  udKC_Unknown, // 264   SDL_SCANCODE_WWW,
  udKC_Unknown, // 265   SDL_SCANCODE_MAIL,
  udKC_Unknown, // 266   SDL_SCANCODE_CALCULATOR,
  udKC_Unknown, // 267   SDL_SCANCODE_COMPUTER,
  udKC_Unknown, // 268   SDL_SCANCODE_AC_SEARCH,
  udKC_Unknown, // 269   SDL_SCANCODE_AC_HOME,
  udKC_Unknown, // 270   SDL_SCANCODE_AC_BACK,
  udKC_Unknown, // 271   SDL_SCANCODE_AC_FORWARD,
  udKC_Unknown, // 272   SDL_SCANCODE_AC_STOP,
  udKC_Unknown, // 273   SDL_SCANCODE_AC_REFRESH,
  udKC_Unknown, // 274   SDL_SCANCODE_AC_BOOKMARKS,
  udKC_Unknown, // 275   SDL_SCANCODE_BRIGHTNESSDOWN,
  udKC_Unknown, // 276   SDL_SCANCODE_BRIGHTNESSUP,
  udKC_Unknown, // 277   SDL_SCANCODE_DISPLAYSWITCH,
  udKC_Unknown, // 278   SDL_SCANCODE_KBDILLUMTOGGLE,
  udKC_Unknown, // 279   SDL_SCANCODE_KBDILLUMDOWN,
  udKC_Unknown, // 280   SDL_SCANCODE_KBDILLUMUP,
  udKC_Unknown, // 281   SDL_SCANCODE_EJECT,
  udKC_Unknown  // 282   SDL_SCANCODE_SLEEP
};

// --------------------------------------------------------
// Author: Manu Evans, March 2015
void udInput_InitInternal()
{

}

// --------------------------------------------------------
// Author: Manu Evans, March 2015
void udInput_UpdateInternal()
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
    if(sdlScanCodeToUDKey[i] == udKC_Unknown)
      continue;
    input.keys[0][sdlScanCodeToUDKey[i]] = (unsigned char)state[i];
  }

  // poll mouse
  int x, y;
  Uint32 buttons = SDL_GetMouseState(&x, &y);
  input.mouse[0][udMC_XAbsolute] = (float)x;
  input.mouse[0][udMC_YAbsolute] = (float)y;
  input.mouse[0][udMC_XDelta] = input.mouse[0][udMC_XAbsolute] - prev.mouse[0][udMC_XAbsolute];
  input.mouse[0][udMC_YDelta] = input.mouse[0][udMC_YAbsolute] - prev.mouse[0][udMC_YAbsolute];
  input.mouse[0][udMC_LeftButton] = (buttons & SDL_BUTTON_LMASK) ? 1.f : 0.f;
  input.mouse[0][udMC_RightButton] = (buttons & SDL_BUTTON_RMASK) ? 1.f : 0.f;
  input.mouse[0][udMC_MiddleButton] = (buttons & SDL_BUTTON_MMASK) ? 1.f : 0.f;
  input.mouse[0][udMC_Button4] = (buttons & SDL_BUTTON_X1MASK) ? 1.f : 0.f;
  input.mouse[0][udMC_Button5] = (buttons & SDL_BUTTON_X2MASK) ? 1.f : 0.f;

  // poll gamepads
  //...
}

#endif
