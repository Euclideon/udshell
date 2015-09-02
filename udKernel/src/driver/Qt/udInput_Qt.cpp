#include "hal/driver.h"

#if UDINPUT_DRIVER == UDDRIVER_QT

#include "hal/input_internal.h"

#include <Qt>

// --------------------------------------------------------
// Author: Manu Evans, March 2015
void udInput_InitInternal()
{

}

// --------------------------------------------------------
// Author: Manu Evans, March 2015
void udInput_UpdateInternal()
{
//  InputState &input = gInputState[gCurrentInputState];
//  InputState &prev = gInputState[1 - gCurrentInputState];

  // poll keyboard
  //...

  // poll mouse
  //...

  // poll gamepads
  //...
}

static unsigned char qtKeyToUDKeyLower[] =
{
  udKC_Space, // Qt::Key_Space	0x20
  udKC_Unknown, // Qt::Key_Exclam	0x21
  udKC_Unknown, // Qt::Key_QuoteDbl	0x22
  udKC_Unknown, // Qt::Key_NumberSign	0x23
  udKC_Unknown, // Qt::Key_Dollar	0x24
  udKC_Unknown, // Qt::Key_Percent	0x25
  udKC_Unknown, // Qt::Key_Ampersand	0x26
  udKC_Apostrophe, // Qt::Key_Apostrophe	0x27
  udKC_Unknown, // Qt::Key_ParenLeft	0x28
  udKC_Unknown, // Qt::Key_ParenRight	0x29
  udKC_NumpadMultiply, // Qt::Key_Asterisk	0x2a
  udKC_NumpadPlus, // Qt::Key_Plus	0x2b
  udKC_Comma, // Qt::Key_Comma	0x2c
  udKC_NumpadMinus, // Qt::Key_Minus	0x2d
  udKC_Period, // Qt::Key_Period	0x2e
  udKC_ForwardSlash, // Qt::Key_Slash	0x2f
  udKC_0, // Qt::Key_0	0x30
  udKC_1, // Qt::Key_1	0x31
  udKC_2, // Qt::Key_2	0x32
  udKC_3, // Qt::Key_3	0x33
  udKC_4, // Qt::Key_4	0x34
  udKC_5, // Qt::Key_5	0x35
  udKC_6, // Qt::Key_6	0x36
  udKC_7, // Qt::Key_7	0x37
  udKC_8, // Qt::Key_8	0x38
  udKC_9, // Qt::Key_9	0x39
  udKC_Unknown, // Qt::Key_Colon	0x3a
  udKC_Semicolon, // Qt::Key_Semicolon	0x3b
  udKC_Unknown, // Qt::Key_Less	0x3c
  udKC_Equals, // Qt::Key_Equal	0x3d
  udKC_Unknown, // Qt::Key_Greater	0x3e
  udKC_Unknown, // Qt::Key_Question	0x3f
  udKC_Unknown, // Qt::Key_At	0x40
  udKC_A, // Qt::Key_A	0x41
  udKC_B, // Qt::Key_B	0x42
  udKC_C, // Qt::Key_C	0x43
  udKC_D, // Qt::Key_D	0x44
  udKC_E, // Qt::Key_E	0x45
  udKC_F, // Qt::Key_F	0x46
  udKC_G, // Qt::Key_G	0x47
  udKC_H, // Qt::Key_H	0x48
  udKC_I, // Qt::Key_I	0x49
  udKC_J, // Qt::Key_J	0x4a
  udKC_K, // Qt::Key_K	0x4b
  udKC_L, // Qt::Key_L	0x4c
  udKC_M, // Qt::Key_M	0x4d
  udKC_N, // Qt::Key_N	0x4e
  udKC_O, // Qt::Key_O	0x4f
  udKC_P, // Qt::Key_P	0x50
  udKC_Q, // Qt::Key_Q	0x51
  udKC_R, // Qt::Key_R	0x52
  udKC_S, // Qt::Key_S	0x53
  udKC_T, // Qt::Key_T	0x54
  udKC_U, // Qt::Key_U	0x55
  udKC_V, // Qt::Key_V	0x56
  udKC_W, // Qt::Key_W	0x57
  udKC_X, // Qt::Key_X	0x58
  udKC_Y, // Qt::Key_Y	0x59
  udKC_Z, // Qt::Key_Z	0x5a
  udKC_LeftBracket, // Qt::Key_BracketLeft	0x5b
  udKC_BackSlash, // Qt::Key_Backslash	0x5c
  udKC_RightBracket, // Qt::Key_BracketRight	0x5d
  udKC_Unknown, // Qt::Key_AsciiCircum	0x5e
  udKC_Unknown, // Qt::Key_Underscore	0x5f
  udKC_Unknown, // Qt::Key_QuoteLeft	0x60

  udKC_Unknown, // 61
  udKC_Unknown, // 62
  udKC_Unknown, // 63
  udKC_Unknown, // 64
  udKC_Unknown, // 65
  udKC_Unknown, // 66
  udKC_Unknown, // 67
  udKC_Unknown, // 68
  udKC_Unknown, // 69
  udKC_Unknown, // 6a
  udKC_Unknown, // 6b
  udKC_Unknown, // 6c
  udKC_Unknown, // 6d
  udKC_Unknown, // 6e
  udKC_Unknown, // 6f
  udKC_Unknown, // 70
  udKC_Unknown, // 71
  udKC_Unknown, // 72
  udKC_Unknown, // 73
  udKC_Unknown, // 74
  udKC_Unknown, // 75
  udKC_Unknown, // 76
  udKC_Unknown, // 77
  udKC_Unknown, // 78
  udKC_Unknown, // 79
  udKC_Unknown, // 7a

  udKC_Unknown, // Qt::Key_BraceLeft	0x7b
  udKC_Unknown, // Qt::Key_Bar	0x7c
  udKC_Unknown, // Qt::Key_BraceRight	0x7d
  udKC_Unknown, // Qt::Key_AsciiTilde	0x7e

  udKC_Unknown, // 7f
  udKC_Unknown, // 80
  udKC_Unknown, // 81
  udKC_Unknown, // 82
  udKC_Unknown, // 83
  udKC_Unknown, // 84
  udKC_Unknown, // 85
  udKC_Unknown, // 86
  udKC_Unknown, // 87
  udKC_Unknown, // 88
  udKC_Unknown, // 89
  udKC_Unknown, // 8a
  udKC_Unknown, // 8b
  udKC_Unknown, // 8c
  udKC_Unknown, // 8d
  udKC_Unknown, // 8e
  udKC_Unknown, // 8f

  udKC_Unknown, // Qt::Key_nobreakspace	0x0a0
  udKC_Unknown, // Qt::Key_exclamdown	0x0a1
  udKC_Unknown, // Qt::Key_cent	0x0a2
  udKC_Unknown, // Qt::Key_sterling	0x0a3
  udKC_Unknown, // Qt::Key_currency	0x0a4
  udKC_Unknown, // Qt::Key_yen	0x0a5
  udKC_Unknown, // Qt::Key_brokenbar	0x0a6
  udKC_Unknown, // Qt::Key_section	0x0a7
  udKC_Unknown, // Qt::Key_diaeresis	0x0a8
  udKC_Unknown, // Qt::Key_copyright	0x0a9
  udKC_Unknown, // Qt::Key_ordfeminine	0x0aa
  udKC_Unknown, // Qt::Key_guillemotleft	0x0ab
  udKC_Unknown, // Qt::Key_notsign	0x0ac
  udKC_Hyphen, // Qt::Key_hyphen	0x0ad
  udKC_Unknown, // Qt::Key_registered	0x0ae
  udKC_Unknown, // Qt::Key_macron	0x0af
  udKC_Unknown, // Qt::Key_degree	0x0b0
  udKC_Unknown, // Qt::Key_plusminus	0x0b1
  udKC_Unknown, // Qt::Key_twosuperior	0x0b2
  udKC_Unknown, // Qt::Key_threesuperior	0x0b3
  udKC_Unknown, // Qt::Key_acute	0x0b4
  udKC_Unknown, // Qt::Key_mu	0x0b5
  udKC_Unknown, // Qt::Key_paragraph	0x0b6
  udKC_Unknown, // Qt::Key_periodcentered	0x0b7
  udKC_Unknown, // Qt::Key_cedilla	0x0b8
  udKC_Unknown, // Qt::Key_onesuperior	0x0b9
  udKC_Unknown, // Qt::Key_masculine	0x0ba
  udKC_Unknown, // Qt::Key_guillemotright	0x0bb
  udKC_Unknown, // Qt::Key_onequarter	0x0bc
  udKC_Unknown, // Qt::Key_onehalf	0x0bd
  udKC_Unknown, // Qt::Key_threequarters	0x0be
  udKC_Unknown, // Qt::Key_questiondown	0x0bf
  udKC_Unknown, // Qt::Key_Agrave	0x0c0
  udKC_Unknown, // Qt::Key_Aacute	0x0c1
  udKC_Unknown, // Qt::Key_Acircumflex	0x0c2
  udKC_Unknown, // Qt::Key_Atilde	0x0c3
  udKC_Unknown, // Qt::Key_Adiaeresis	0x0c4
  udKC_Unknown, // Qt::Key_Aring	0x0c5
  udKC_Unknown, // Qt::Key_AE	0x0c6
  udKC_Unknown, // Qt::Key_Ccedilla	0x0c7
  udKC_Unknown, // Qt::Key_Egrave	0x0c8
  udKC_Unknown, // Qt::Key_Eacute	0x0c9
  udKC_Unknown, // Qt::Key_Ecircumflex	0x0ca
  udKC_Unknown, // Qt::Key_Ediaeresis	0x0cb
  udKC_Unknown, // Qt::Key_Igrave	0x0cc
  udKC_Unknown, // Qt::Key_Iacute	0x0cd
  udKC_Unknown, // Qt::Key_Icircumflex	0x0ce
  udKC_Unknown, // Qt::Key_Idiaeresis	0x0cf
  udKC_Unknown, // Qt::Key_ETH	0x0d0
  udKC_Unknown, // Qt::Key_Ntilde	0x0d1
  udKC_Unknown, // Qt::Key_Ograve	0x0d2
  udKC_Unknown, // Qt::Key_Oacute	0x0d3
  udKC_Unknown, // Qt::Key_Ocircumflex	0x0d4
  udKC_Unknown, // Qt::Key_Otilde	0x0d5
  udKC_Unknown, // Qt::Key_Odiaeresis	0x0d6
  udKC_NumpadMultiply, // Qt::Key_multiply	0x0d7
  udKC_Unknown, // Qt::Key_Ooblique	0x0d8
  udKC_Unknown, // Qt::Key_Ugrave	0x0d9
  udKC_Unknown, // Qt::Key_Uacute	0x0da
  udKC_Unknown, // Qt::Key_Ucircumflex	0x0db
  udKC_Unknown, // Qt::Key_Udiaeresis	0x0dc
  udKC_Unknown, // Qt::Key_Yacute	0x0dd
  udKC_Unknown, // Qt::Key_THORN	0x0de
  udKC_Unknown, // Qt::Key_ssharp	0x0df

  udKC_Unknown, // e0
  udKC_Unknown, // e1
  udKC_Unknown, // e2
  udKC_Unknown, // e3
  udKC_Unknown, // e4
  udKC_Unknown, // e5
  udKC_Unknown, // e6
  udKC_Unknown, // e7
  udKC_Unknown, // e8
  udKC_Unknown, // e9
  udKC_Unknown, // ea
  udKC_Unknown, // eb
  udKC_Unknown, // ec
  udKC_Unknown, // ed
  udKC_Unknown, // ee
  udKC_Unknown, // ef
  udKC_Unknown, // f0
  udKC_Unknown, // f1
  udKC_Unknown, // f2
  udKC_Unknown, // f3
  udKC_Unknown, // f4
  udKC_Unknown, // f5
  udKC_Unknown, // f6

  udKC_NumpadDivide, // Qt::Key_division	0x0f7

  udKC_Unknown, // f8
  udKC_Unknown, // f9
  udKC_Unknown, // fa
  udKC_Unknown, // fb
  udKC_Unknown, // fc
  udKC_Unknown, // fd
  udKC_Unknown, // fe

  udKC_Unknown, // Qt::Key_ydiaeresis	0x0ff
};

static unsigned char qtKeyToUDKeyHighBit[] =
{
  udKC_Escape, // Qt::Key_Escape	0x01000000
  udKC_Tab, // Qt::Key_Tab	0x01000001
  udKC_Unknown, // Qt::Key_Backtab	0x01000002
  udKC_Backspace, // Qt::Key_Backspace	0x01000003
  udKC_Enter, // Qt::Key_Return	0x01000004
  udKC_Enter, // Qt::Key_Enter	0x01000005	Typically located on the keypad.
  udKC_Insert, // Qt::Key_Insert	0x01000006
  udKC_Delete, // Qt::Key_Delete	0x01000007
  udKC_Pause, // Qt::Key_Pause	0x01000008	The Pause/Break key(Note: Not related to pausing media)
  udKC_PrintScreen, // Qt::Key_Print	0x01000009
  udKC_Unknown, // Qt::Key_SysReq	0x0100000a
  udKC_Unknown, // Qt::Key_Clear	0x0100000b

  udKC_Unknown, // 0c
  udKC_Unknown, // 0d
  udKC_Unknown, // 0e
  udKC_Unknown, // 0f

  udKC_Home, // Qt::Key_Home	0x01000010
  udKC_End, // Qt::Key_End	0x01000011
  udKC_Left, // Qt::Key_Left	0x01000012
  udKC_Up, // Qt::Key_Up	0x01000013
  udKC_Right, // Qt::Key_Right	0x01000014
  udKC_Down, // Qt::Key_Down	0x01000015
  udKC_PageUp, // Qt::Key_PageUp	0x01000016
  udKC_PageDown, // Qt::Key_PageDown	0x01000017

  udKC_Unknown, // 18
  udKC_Unknown, // 19
  udKC_Unknown, // 1a
  udKC_Unknown, // 1b
  udKC_Unknown, // 1c
  udKC_Unknown, // 1d
  udKC_Unknown, // 1e
  udKC_Unknown, // 1f

  udKC_LShift, // Qt::Key_Shift	0x01000020
  udKC_LCtrl, // Qt::Key_Control	0x01000021	On OS X, this corresponds to the Command keys.
  udKC_LWin, // Qt::Key_Meta	0x01000022	On OS X, this corresponds to the Control keys.On Windows keyboards, this key is mapped to the Windows key.
  udKC_LAlt, // Qt::Key_Alt	0x01000023
  udKC_CapsLock, // Qt::Key_CapsLock	0x01000024
  udKC_NumLock, // Qt::Key_NumLock	0x01000025
  udKC_ScrollLock, // Qt::Key_ScrollLock	0x01000026

  udKC_Unknown, // 27
  udKC_Unknown, // 28
  udKC_Unknown, // 29
  udKC_Unknown, // 2a
  udKC_Unknown, // 2b
  udKC_Unknown, // 2c
  udKC_Unknown, // 2d
  udKC_Unknown, // 2e
  udKC_Unknown, // 2f

  udKC_F1, // Qt::Key_F1	0x01000030
  udKC_F2, // Qt::Key_F2	0x01000031
  udKC_F3, // Qt::Key_F3	0x01000032
  udKC_F4, // Qt::Key_F4	0x01000033
  udKC_F5, // Qt::Key_F5	0x01000034
  udKC_F6, // Qt::Key_F6	0x01000035
  udKC_F7, // Qt::Key_F7	0x01000036
  udKC_F8, // Qt::Key_F8	0x01000037
  udKC_F9, // Qt::Key_F9	0x01000038
  udKC_F10, // Qt::Key_F10	0x01000039
  udKC_F11, // Qt::Key_F11	0x0100003a
  udKC_F12, // Qt::Key_F12	0x0100003b
  udKC_Unknown, // Qt::Key_F13	0x0100003c
  udKC_Unknown, // Qt::Key_F14	0x0100003d
  udKC_Unknown, // Qt::Key_F15	0x0100003e
  udKC_Unknown, // Qt::Key_F16	0x0100003f
  udKC_Unknown, // Qt::Key_F17	0x01000040
  udKC_Unknown, // Qt::Key_F18	0x01000041
  udKC_Unknown, // Qt::Key_F19	0x01000042
  udKC_Unknown, // Qt::Key_F20	0x01000043
  udKC_Unknown, // Qt::Key_F21	0x01000044
  udKC_Unknown, // Qt::Key_F22	0x01000045
  udKC_Unknown, // Qt::Key_F23	0x01000046
  udKC_Unknown, // Qt::Key_F24	0x01000047
  udKC_Unknown, // Qt::Key_F25	0x01000048
  udKC_Unknown, // Qt::Key_F26	0x01000049
  udKC_Unknown, // Qt::Key_F27	0x0100004a
  udKC_Unknown, // Qt::Key_F28	0x0100004b
  udKC_Unknown, // Qt::Key_F29	0x0100004c
  udKC_Unknown, // Qt::Key_F30	0x0100004d
  udKC_Unknown, // Qt::Key_F31	0x0100004e
  udKC_Unknown, // Qt::Key_F32	0x0100004f
  udKC_Unknown, // Qt::Key_F33	0x01000050
  udKC_Unknown, // Qt::Key_F34	0x01000051
  udKC_Unknown, // Qt::Key_F35	0x01000052
  udKC_Unknown, // Qt::Key_Super_L	0x01000053
  udKC_Unknown, // Qt::Key_Super_R	0x01000054
  udKC_Menu, // Qt::Key_Menu	0x01000055
  udKC_Unknown, // Qt::Key_Hyper_L	0x01000056
  udKC_Unknown, // Qt::Key_Hyper_R	0x01000057
  udKC_Unknown, // Qt::Key_Help	0x01000058
  udKC_Unknown, // Qt::Key_Direction_L	0x01000059

  udKC_Unknown, // 5a
  udKC_Unknown, // 5b
  udKC_Unknown, // 5c
  udKC_Unknown, // 5d
  udKC_Unknown, // 5e
  udKC_Unknown, // 5f

  udKC_Unknown, // Qt::Key_Direction_R	0x01000060
  udKC_Unknown, // Qt::Key_Back	0x01000061
  udKC_Unknown, // Qt::Key_Forward	0x01000062
  udKC_Unknown, // Qt::Key_Stop	0x01000063
  udKC_Unknown, // Qt::Key_Refresh	0x01000064

  udKC_Unknown, // 65
  udKC_Unknown, // 66
  udKC_Unknown, // 67
  udKC_Unknown, // 68
  udKC_Unknown, // 69
  udKC_Unknown, // 6a
  udKC_Unknown, // 6b
  udKC_Unknown, // 6c
  udKC_Unknown, // 6d
  udKC_Unknown, // 6e
  udKC_Unknown, // 6f

  udKC_Unknown, // Qt::Key_VolumeDown	0x01000070
  udKC_Unknown, // Qt::Key_VolumeMute	0x01000071
  udKC_Unknown, // Qt::Key_VolumeUp	0x01000072
  udKC_Unknown, // Qt::Key_BassBoost	0x01000073
  udKC_Unknown, // Qt::Key_BassUp	0x01000074
  udKC_Unknown, // Qt::Key_BassDown	0x01000075
  udKC_Unknown, // Qt::Key_TrebleUp	0x01000076
  udKC_Unknown, // Qt::Key_TrebleDown	0x01000077

  udKC_Unknown, // 78
  udKC_Unknown, // 79
  udKC_Unknown, // 7a
  udKC_Unknown, // 7b
  udKC_Unknown, // 7c
  udKC_Unknown, // 7d
  udKC_Unknown, // 7e
  udKC_Unknown, // 7f

  udKC_Unknown, // Qt::Key_MediaPlay	0x01000080	A key setting the state of the media player to play
  udKC_Unknown, // Qt::Key_MediaStop	0x01000081	A key setting the state of the media player to stop
  udKC_Unknown, // Qt::Key_MediaPrevious	0x01000082
  udKC_Unknown, // Qt::Key_MediaNext	0x01000083
  udKC_Unknown, // Qt::Key_MediaRecord	0x01000084
  udKC_Unknown, // Qt::Key_MediaPause	0x1000085	A key setting the state of the media player to pause(Note: not the pause/break key)
  udKC_Unknown, // Qt::Key_MediaTogglePlayPause	0x1000086	A key to toggle the play/pause state in the media player(rather than setting an absolute state)

  udKC_Unknown, // 87
  udKC_Unknown, // 88
  udKC_Unknown, // 89
  udKC_Unknown, // 8a
  udKC_Unknown, // 8b
  udKC_Unknown, // 8c
  udKC_Unknown, // 8d
  udKC_Unknown, // 8e
  udKC_Unknown, // 8f

  udKC_Unknown, // Qt::Key_HomePage	0x01000090
  udKC_Unknown, // Qt::Key_Favorites	0x01000091
  udKC_Unknown, // Qt::Key_Search	0x01000092
  udKC_Unknown, // Qt::Key_Standby	0x01000093
  udKC_Unknown, // Qt::Key_OpenUrl	0x01000094

  udKC_Unknown, // 95
  udKC_Unknown, // 96
  udKC_Unknown, // 97
  udKC_Unknown, // 98
  udKC_Unknown, // 99
  udKC_Unknown, // 9a
  udKC_Unknown, // 9b
  udKC_Unknown, // 9c
  udKC_Unknown, // 9d
  udKC_Unknown, // 9e
  udKC_Unknown, // 9f

  udKC_Unknown, // Qt::Key_LaunchMail	0x010000a0
  udKC_Unknown, // Qt::Key_LaunchMedia	0x010000a1
  udKC_Unknown, // Qt::Key_Launch0	0x010000a2	On X11 this key is mapped to "My Computer" (XF86XK_MyComputer)key for legacy reasons.
  udKC_Unknown, // Qt::Key_Launch1	0x010000a3	On X11 this key is mapped to "Calculator" (XF86XK_Calculator)key for legacy reasons.
  udKC_Unknown, // Qt::Key_Launch2	0x010000a4	On X11 this key is mapped to XF86XK_Launch0 key for legacy reasons.
  udKC_Unknown, // Qt::Key_Launch3	0x010000a5	On X11 this key is mapped to XF86XK_Launch1 key for legacy reasons.
  udKC_Unknown, // Qt::Key_Launch4	0x010000a6	On X11 this key is mapped to XF86XK_Launch2 key for legacy reasons.
  udKC_Unknown, // Qt::Key_Launch5	0x010000a7	On X11 this key is mapped to XF86XK_Launch3 key for legacy reasons.
  udKC_Unknown, // Qt::Key_Launch6	0x010000a8	On X11 this key is mapped to XF86XK_Launch4 key for legacy reasons.
  udKC_Unknown, // Qt::Key_Launch7	0x010000a9	On X11 this key is mapped to XF86XK_Launch5 key for legacy reasons.
  udKC_Unknown, // Qt::Key_Launch8	0x010000aa	On X11 this key is mapped to XF86XK_Launch6 key for legacy reasons.
  udKC_Unknown, // Qt::Key_Launch9	0x010000ab	On X11 this key is mapped to XF86XK_Launch7 key for legacy reasons.
  udKC_Unknown, // Qt::Key_LaunchA	0x010000ac	On X11 this key is mapped to XF86XK_Launch8 key for legacy reasons.
  udKC_Unknown, // Qt::Key_LaunchB	0x010000ad	On X11 this key is mapped to XF86XK_Launch9 key for legacy reasons.
  udKC_Unknown, // Qt::Key_LaunchC	0x010000ae	On X11 this key is mapped to XF86XK_LaunchA key for legacy reasons.
  udKC_Unknown, // Qt::Key_LaunchD	0x010000af	On X11 this key is mapped to XF86XK_LaunchB key for legacy reasons.
  udKC_Unknown, // Qt::Key_LaunchE	0x010000b0	On X11 this key is mapped to XF86XK_LaunchC key for legacy reasons.
  udKC_Unknown, // Qt::Key_LaunchF	0x010000b1	On X11 this key is mapped to XF86XK_LaunchD key for legacy reasons.
  udKC_Unknown, // Qt::Key_MonBrightnessUp	0x010000b2
  udKC_Unknown, // Qt::Key_MonBrightnessDown	0x010000b3
  udKC_Unknown, // Qt::Key_KeyboardLightOnOff	0x010000b4
  udKC_Unknown, // Qt::Key_KeyboardBrightnessUp	0x010000b5
  udKC_Unknown, // Qt::Key_KeyboardBrightnessDown	0x010000b6
  udKC_Unknown, // Qt::Key_PowerOff	0x010000b7
  udKC_Unknown, // Qt::Key_WakeUp	0x010000b8
  udKC_Unknown, // Qt::Key_Eject	0x010000b9
  udKC_Unknown, // Qt::Key_ScreenSaver	0x010000ba
  udKC_Unknown, // Qt::Key_WWW	0x010000bb
  udKC_Unknown, // Qt::Key_Memo	0x010000bc
  udKC_Unknown, // Qt::Key_LightBulb	0x010000bd
  udKC_Unknown, // Qt::Key_Shop	0x010000be
  udKC_Unknown, // Qt::Key_History	0x010000bf
  udKC_Unknown, // Qt::Key_AddFavorite	0x010000c0
  udKC_Unknown, // Qt::Key_HotLinks	0x010000c1
  udKC_Unknown, // Qt::Key_BrightnessAdjust	0x010000c2
  udKC_Unknown, // Qt::Key_Finance	0x010000c3
  udKC_Unknown, // Qt::Key_Community	0x010000c4
  udKC_Unknown, // Qt::Key_AudioRewind	0x010000c5
  udKC_Unknown, // Qt::Key_BackForward	0x010000c6
  udKC_Unknown, // Qt::Key_ApplicationLeft	0x010000c7
  udKC_Unknown, // Qt::Key_ApplicationRight	0x010000c8
  udKC_Unknown, // Qt::Key_Book	0x010000c9
  udKC_Unknown, // Qt::Key_CD	0x010000ca
  udKC_Unknown, // Qt::Key_Calculator	0x010000cb	On X11 this key is not mapped for legacy reasons.Use Qt::Key_Launch1 instead.
  udKC_Unknown, // Qt::Key_ToDoList	0x010000cc
  udKC_Unknown, // Qt::Key_ClearGrab	0x010000cd
  udKC_Unknown, // Qt::Key_Close	0x010000ce
  udKC_Unknown, // Qt::Key_Copy	0x010000cf
  udKC_Unknown, // Qt::Key_Cut	0x010000d0
  udKC_Unknown, // Qt::Key_Display	0x010000d1
  udKC_Unknown, // Qt::Key_DOS	0x010000d2
  udKC_Unknown, // Qt::Key_Documents	0x010000d3
  udKC_Unknown, // Qt::Key_Excel	0x010000d4
  udKC_Unknown, // Qt::Key_Explorer	0x010000d5
  udKC_Unknown, // Qt::Key_Game	0x010000d6
  udKC_Unknown, // Qt::Key_Go	0x010000d7
  udKC_Unknown, // Qt::Key_iTouch	0x010000d8
  udKC_Unknown, // Qt::Key_LogOff	0x010000d9
  udKC_Unknown, // Qt::Key_Market	0x010000da
  udKC_Unknown, // Qt::Key_Meeting	0x010000db
  udKC_Unknown, // Qt::Key_MenuKB	0x010000dc
  udKC_Unknown, // Qt::Key_MenuPB	0x010000dd
  udKC_Unknown, // Qt::Key_MySites	0x010000de
  udKC_Unknown, // Qt::Key_News	0x010000df
  udKC_Unknown, // Qt::Key_OfficeHome	0x010000e0
  udKC_Unknown, // Qt::Key_Option	0x010000e1
  udKC_Unknown, // Qt::Key_Paste	0x010000e2
  udKC_Unknown, // Qt::Key_Phone	0x010000e3
  udKC_Unknown, // Qt::Key_Calendar	0x010000e4
  udKC_Unknown, // Qt::Key_Reply	0x010000e5
  udKC_Unknown, // Qt::Key_Reload	0x010000e6
  udKC_Unknown, // Qt::Key_RotateWindows	0x010000e7
  udKC_Unknown, // Qt::Key_RotationPB	0x010000e8
  udKC_Unknown, // Qt::Key_RotationKB	0x010000e9
  udKC_Unknown, // Qt::Key_Save	0x010000ea
  udKC_Unknown, // Qt::Key_Send	0x010000eb
  udKC_Unknown, // Qt::Key_Spell	0x010000ec
  udKC_Unknown, // Qt::Key_SplitScreen	0x010000ed
  udKC_Unknown, // Qt::Key_Support	0x010000ee
  udKC_Unknown, // Qt::Key_TaskPane	0x010000ef
  udKC_Unknown, // Qt::Key_Terminal	0x010000f0
  udKC_Unknown, // Qt::Key_Tools	0x010000f1
  udKC_Unknown, // Qt::Key_Travel	0x010000f2
  udKC_Unknown, // Qt::Key_Video	0x010000f3
  udKC_Unknown, // Qt::Key_Word	0x010000f4
  udKC_Unknown, // Qt::Key_Xfer	0x010000f5
  udKC_Unknown, // Qt::Key_ZoomIn	0x010000f6
  udKC_Unknown, // Qt::Key_ZoomOut	0x010000f7
  udKC_Unknown, // Qt::Key_Away	0x010000f8
  udKC_Unknown, // Qt::Key_Messenger	0x010000f9
  udKC_Unknown, // Qt::Key_WebCam	0x010000fa
  udKC_Unknown, // Qt::Key_MailForward	0x010000fb
  udKC_Unknown, // Qt::Key_Pictures	0x010000fc
  udKC_Unknown, // Qt::Key_Music	0x010000fd
  udKC_Unknown, // Qt::Key_Battery	0x010000fe
  udKC_Unknown, // Qt::Key_Bluetooth	0x010000ff
  udKC_Unknown, // Qt::Key_WLAN	0x01000100
  udKC_Unknown, // Qt::Key_UWB	0x01000101
  udKC_Unknown, // Qt::Key_AudioForward	0x01000102
  udKC_Unknown, // Qt::Key_AudioRepeat	0x01000103
  udKC_Unknown, // Qt::Key_AudioRandomPlay	0x01000104
  udKC_Unknown, // Qt::Key_Subtitle	0x01000105
  udKC_Unknown, // Qt::Key_AudioCycleTrack	0x01000106
  udKC_Unknown, // Qt::Key_Time	0x01000107
  udKC_Unknown, // Qt::Key_Hibernate	0x01000108
  udKC_Unknown, // Qt::Key_View	0x01000109
  udKC_Unknown, // Qt::Key_TopMenu	0x0100010a
  udKC_Unknown, // Qt::Key_PowerDown	0x0100010b
  udKC_Unknown, // Qt::Key_Suspend	0x0100010c
  udKC_Unknown, // Qt::Key_ContrastAdjust	0x0100010d
  udKC_Unknown, // Qt::Key_LaunchG	0x0100010e	On X11 this key is mapped to XF86XK_LaunchE key for legacy reasons.
  udKC_Unknown, // Qt::Key_LaunchH	0x0100010f	On X11 this key is mapped to XF86XK_LaunchF key for legacy reasons.
  udKC_Unknown, // Qt::Key_TouchpadToggle	0x01000110
  udKC_Unknown, // Qt::Key_TouchpadOn	0x01000111
  udKC_Unknown, // Qt::Key_TouchpadOff	0x01000112
  udKC_Unknown, // Qt::Key_MicMute	0x01000113
  udKC_Unknown, // Qt::Key_Red	0x01000114
  udKC_Unknown, // Qt::Key_Green	0x01000115
  udKC_Unknown, // Qt::Key_Yellow	0x01000116
  udKC_Unknown, // Qt::Key_Blue	0x01000117
  udKC_Unknown, // Qt::Key_ChannelUp	0x01000118
  udKC_Unknown, // Qt::Key_ChannelDown	0x01000119
};

udKeyCode qtKeyToUDKey(Qt::Key qk)
{
  if ((qk & ~0xFF) == 0)
    return (udKeyCode)qtKeyToUDKeyLower[qk - 0x20];
  else if ((qk & 0xFFFFFF00) == 0x01000000)
    return (udKeyCode)qtKeyToUDKeyHighBit[qk & 0xFFF];
  else
  {
//    udKC_RAlt, // Qt::Key_AltGr	0x01001103	On Windows, when the KeyDown event for this key is sent, the Ctrl+Alt modifiers are also set.
    // Qt::Key_Multi_key	0x01001120
    // Qt::Key_Codeinput	0x01001137
    // Qt::Key_SingleCandidate	0x0100113c
    // Qt::Key_MultipleCandidate	0x0100113d
    // Qt::Key_PreviousCandidate	0x0100113e
    // Qt::Key_Mode_switch	0x0100117e
    // Qt::Key_Kanji	0x01001121
    // Qt::Key_Muhenkan	0x01001122
    // Qt::Key_Henkan	0x01001123
    // Qt::Key_Romaji	0x01001124
    // Qt::Key_Hiragana	0x01001125
    // Qt::Key_Katakana	0x01001126
    // Qt::Key_Hiragana_Katakana	0x01001127
    // Qt::Key_Zenkaku	0x01001128
    // Qt::Key_Hankaku	0x01001129
    // Qt::Key_Zenkaku_Hankaku	0x0100112a
    // Qt::Key_Touroku	0x0100112b
    // Qt::Key_Massyo	0x0100112c
    // Qt::Key_Kana_Lock	0x0100112d
    // Qt::Key_Kana_Shift	0x0100112e
    // Qt::Key_Eisu_Shift	0x0100112f
    // Qt::Key_Eisu_toggle	0x01001130
    // Qt::Key_Hangul	0x01001131
    // Qt::Key_Hangul_Start	0x01001132
    // Qt::Key_Hangul_End	0x01001133
    // Qt::Key_Hangul_Hanja	0x01001134
    // Qt::Key_Hangul_Jamo	0x01001135
    // Qt::Key_Hangul_Romaja	0x01001136
    // Qt::Key_Hangul_Jeonja	0x01001138
    // Qt::Key_Hangul_Banja	0x01001139
    // Qt::Key_Hangul_PreHanja	0x0100113a
    // Qt::Key_Hangul_PostHanja	0x0100113b
    // Qt::Key_Hangul_Special	0x0100113f
    // Qt::Key_Dead_Grave	0x01001250
    // Qt::Key_Dead_Acute	0x01001251
    // Qt::Key_Dead_Circumflex	0x01001252
    // Qt::Key_Dead_Tilde	0x01001253
    // Qt::Key_Dead_Macron	0x01001254
    // Qt::Key_Dead_Breve	0x01001255
    // Qt::Key_Dead_Abovedot	0x01001256
    // Qt::Key_Dead_Diaeresis	0x01001257
    // Qt::Key_Dead_Abovering	0x01001258
    // Qt::Key_Dead_Doubleacute	0x01001259
    // Qt::Key_Dead_Caron	0x0100125a
    // Qt::Key_Dead_Cedilla	0x0100125b
    // Qt::Key_Dead_Ogonek	0x0100125c
    // Qt::Key_Dead_Iota	0x0100125d
    // Qt::Key_Dead_Voiced_Sound	0x0100125e
    // Qt::Key_Dead_Semivoiced_Sound	0x0100125f
    // Qt::Key_Dead_Belowdot	0x01001260
    // Qt::Key_Dead_Hook	0x01001261
    // Qt::Key_Dead_Horn	0x01001262

    // Qt::Key_MediaLast	0x0100ffff
    // Qt::Key_unknown	0x01ffffff
    // Qt::Key_Call	0x01100004	A key to answer or initiate a call(see Qt::Key_ToggleCallHangup for a key to toggle current call state)
    // Qt::Key_Camera	0x01100020	A key to activate the camera shutter
    // Qt::Key_CameraFocus	0x01100021	A key to focus the camera
    // Qt::Key_Context1	0x01100000
    // Qt::Key_Context2	0x01100001
    // Qt::Key_Context3	0x01100002
    // Qt::Key_Context4	0x01100003
    // Qt::Key_Flip	0x01100006
    // Qt::Key_Hangup	0x01100005	A key to end an ongoing call(see Qt::Key_ToggleCallHangup for a key to toggle current call state)
    // Qt::Key_No	0x01010002
    // Qt::Key_Select	0x01010000
    // Qt::Key_Yes	0x01010001
    // Qt::Key_ToggleCallHangup	0x01100007	A key to toggle the current call state(ie.either answer, or hangup) depending on current call state
    // Qt::Key_VoiceDial	0x01100008
    // Qt::Key_LastNumberRedial	0x01100009
    // Qt::Key_Execute	0x01020003
    // Qt::Key_Printer	0x01020002
    // Qt::Key_Play	0x01020005
    // Qt::Key_Sleep	0x01020004
    // Qt::Key_Zoom	0x01020006
    // Qt::Key_Cancel	0x01020001

    return udKC_Unknown;
  }
}

#endif
