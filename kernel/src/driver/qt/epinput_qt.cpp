#include "hal/driver.h"

#if EPINPUT_DRIVER == EPDRIVER_QT

#include "hal/input_internal.h"

#include <Qt>

// --------------------------------------------------------
// Author: Manu Evans, March 2015
void epInput_InitInternal()
{

}

// --------------------------------------------------------
// Author: Manu Evans, March 2015
void epInput_UpdateInternal()
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

static unsigned char qtKeyToEPKeyLower[] =
{
  epKC_Space, // Qt::Key_Space	0x20
  epKC_Unknown, // Qt::Key_Exclam	0x21
  epKC_Unknown, // Qt::Key_QuoteDbl	0x22
  epKC_Unknown, // Qt::Key_NumberSign	0x23
  epKC_Unknown, // Qt::Key_Dollar	0x24
  epKC_Unknown, // Qt::Key_Percent	0x25
  epKC_Unknown, // Qt::Key_Ampersand	0x26
  epKC_Apostrophe, // Qt::Key_Apostrophe	0x27
  epKC_Unknown, // Qt::Key_ParenLeft	0x28
  epKC_Unknown, // Qt::Key_ParenRight	0x29
  epKC_NumpadMultiply, // Qt::Key_Asterisk	0x2a
  epKC_NumpadPlus, // Qt::Key_Plus	0x2b
  epKC_Comma, // Qt::Key_Comma	0x2c
  epKC_NumpadMinus, // Qt::Key_Minus	0x2d
  epKC_Period, // Qt::Key_Period	0x2e
  epKC_ForwardSlash, // Qt::Key_Slash	0x2f
  epKC_0, // Qt::Key_0	0x30
  epKC_1, // Qt::Key_1	0x31
  epKC_2, // Qt::Key_2	0x32
  epKC_3, // Qt::Key_3	0x33
  epKC_4, // Qt::Key_4	0x34
  epKC_5, // Qt::Key_5	0x35
  epKC_6, // Qt::Key_6	0x36
  epKC_7, // Qt::Key_7	0x37
  epKC_8, // Qt::Key_8	0x38
  epKC_9, // Qt::Key_9	0x39
  epKC_Unknown, // Qt::Key_Colon	0x3a
  epKC_Semicolon, // Qt::Key_Semicolon	0x3b
  epKC_Unknown, // Qt::Key_Less	0x3c
  epKC_Equals, // Qt::Key_Equal	0x3d
  epKC_Unknown, // Qt::Key_Greater	0x3e
  epKC_Unknown, // Qt::Key_Question	0x3f
  epKC_Unknown, // Qt::Key_At	0x40
  epKC_A, // Qt::Key_A	0x41
  epKC_B, // Qt::Key_B	0x42
  epKC_C, // Qt::Key_C	0x43
  epKC_D, // Qt::Key_D	0x44
  epKC_E, // Qt::Key_E	0x45
  epKC_F, // Qt::Key_F	0x46
  epKC_G, // Qt::Key_G	0x47
  epKC_H, // Qt::Key_H	0x48
  epKC_I, // Qt::Key_I	0x49
  epKC_J, // Qt::Key_J	0x4a
  epKC_K, // Qt::Key_K	0x4b
  epKC_L, // Qt::Key_L	0x4c
  epKC_M, // Qt::Key_M	0x4d
  epKC_N, // Qt::Key_N	0x4e
  epKC_O, // Qt::Key_O	0x4f
  epKC_P, // Qt::Key_P	0x50
  epKC_Q, // Qt::Key_Q	0x51
  epKC_R, // Qt::Key_R	0x52
  epKC_S, // Qt::Key_S	0x53
  epKC_T, // Qt::Key_T	0x54
  epKC_U, // Qt::Key_U	0x55
  epKC_V, // Qt::Key_V	0x56
  epKC_W, // Qt::Key_W	0x57
  epKC_X, // Qt::Key_X	0x58
  epKC_Y, // Qt::Key_Y	0x59
  epKC_Z, // Qt::Key_Z	0x5a
  epKC_LeftBracket, // Qt::Key_BracketLeft	0x5b
  epKC_BackSlash, // Qt::Key_Backslash	0x5c
  epKC_RightBracket, // Qt::Key_BracketRight	0x5d
  epKC_Unknown, // Qt::Key_AsciiCircum	0x5e
  epKC_Unknown, // Qt::Key_Underscore	0x5f
  epKC_Unknown, // Qt::Key_QuoteLeft	0x60

  epKC_Unknown, // 61
  epKC_Unknown, // 62
  epKC_Unknown, // 63
  epKC_Unknown, // 64
  epKC_Unknown, // 65
  epKC_Unknown, // 66
  epKC_Unknown, // 67
  epKC_Unknown, // 68
  epKC_Unknown, // 69
  epKC_Unknown, // 6a
  epKC_Unknown, // 6b
  epKC_Unknown, // 6c
  epKC_Unknown, // 6d
  epKC_Unknown, // 6e
  epKC_Unknown, // 6f
  epKC_Unknown, // 70
  epKC_Unknown, // 71
  epKC_Unknown, // 72
  epKC_Unknown, // 73
  epKC_Unknown, // 74
  epKC_Unknown, // 75
  epKC_Unknown, // 76
  epKC_Unknown, // 77
  epKC_Unknown, // 78
  epKC_Unknown, // 79
  epKC_Unknown, // 7a

  epKC_Unknown, // Qt::Key_BraceLeft	0x7b
  epKC_Unknown, // Qt::Key_Bar	0x7c
  epKC_Unknown, // Qt::Key_BraceRight	0x7d
  epKC_Unknown, // Qt::Key_AsciiTilde	0x7e

  epKC_Unknown, // 7f
  epKC_Unknown, // 80
  epKC_Unknown, // 81
  epKC_Unknown, // 82
  epKC_Unknown, // 83
  epKC_Unknown, // 84
  epKC_Unknown, // 85
  epKC_Unknown, // 86
  epKC_Unknown, // 87
  epKC_Unknown, // 88
  epKC_Unknown, // 89
  epKC_Unknown, // 8a
  epKC_Unknown, // 8b
  epKC_Unknown, // 8c
  epKC_Unknown, // 8d
  epKC_Unknown, // 8e
  epKC_Unknown, // 8f

  epKC_Unknown, // Qt::Key_nobreakspace	0x0a0
  epKC_Unknown, // Qt::Key_exclamdown	0x0a1
  epKC_Unknown, // Qt::Key_cent	0x0a2
  epKC_Unknown, // Qt::Key_sterling	0x0a3
  epKC_Unknown, // Qt::Key_currency	0x0a4
  epKC_Unknown, // Qt::Key_yen	0x0a5
  epKC_Unknown, // Qt::Key_brokenbar	0x0a6
  epKC_Unknown, // Qt::Key_section	0x0a7
  epKC_Unknown, // Qt::Key_diaeresis	0x0a8
  epKC_Unknown, // Qt::Key_copyright	0x0a9
  epKC_Unknown, // Qt::Key_ordfeminine	0x0aa
  epKC_Unknown, // Qt::Key_guillemotleft	0x0ab
  epKC_Unknown, // Qt::Key_notsign	0x0ac
  epKC_Hyphen, // Qt::Key_hyphen	0x0ad
  epKC_Unknown, // Qt::Key_registered	0x0ae
  epKC_Unknown, // Qt::Key_macron	0x0af
  epKC_Unknown, // Qt::Key_degree	0x0b0
  epKC_Unknown, // Qt::Key_plusminus	0x0b1
  epKC_Unknown, // Qt::Key_twosuperior	0x0b2
  epKC_Unknown, // Qt::Key_threesuperior	0x0b3
  epKC_Unknown, // Qt::Key_acute	0x0b4
  epKC_Unknown, // Qt::Key_mu	0x0b5
  epKC_Unknown, // Qt::Key_paragraph	0x0b6
  epKC_Unknown, // Qt::Key_periodcentered	0x0b7
  epKC_Unknown, // Qt::Key_cedilla	0x0b8
  epKC_Unknown, // Qt::Key_onesuperior	0x0b9
  epKC_Unknown, // Qt::Key_masculine	0x0ba
  epKC_Unknown, // Qt::Key_guillemotright	0x0bb
  epKC_Unknown, // Qt::Key_onequarter	0x0bc
  epKC_Unknown, // Qt::Key_onehalf	0x0bd
  epKC_Unknown, // Qt::Key_threequarters	0x0be
  epKC_Unknown, // Qt::Key_questiondown	0x0bf
  epKC_Unknown, // Qt::Key_Agrave	0x0c0
  epKC_Unknown, // Qt::Key_Aacute	0x0c1
  epKC_Unknown, // Qt::Key_Acircumflex	0x0c2
  epKC_Unknown, // Qt::Key_Atilde	0x0c3
  epKC_Unknown, // Qt::Key_Adiaeresis	0x0c4
  epKC_Unknown, // Qt::Key_Aring	0x0c5
  epKC_Unknown, // Qt::Key_AE	0x0c6
  epKC_Unknown, // Qt::Key_Ccedilla	0x0c7
  epKC_Unknown, // Qt::Key_Egrave	0x0c8
  epKC_Unknown, // Qt::Key_Eacute	0x0c9
  epKC_Unknown, // Qt::Key_Ecircumflex	0x0ca
  epKC_Unknown, // Qt::Key_Ediaeresis	0x0cb
  epKC_Unknown, // Qt::Key_Igrave	0x0cc
  epKC_Unknown, // Qt::Key_Iacute	0x0cd
  epKC_Unknown, // Qt::Key_Icircumflex	0x0ce
  epKC_Unknown, // Qt::Key_Idiaeresis	0x0cf
  epKC_Unknown, // Qt::Key_ETH	0x0d0
  epKC_Unknown, // Qt::Key_Ntilde	0x0d1
  epKC_Unknown, // Qt::Key_Ograve	0x0d2
  epKC_Unknown, // Qt::Key_Oacute	0x0d3
  epKC_Unknown, // Qt::Key_Ocircumflex	0x0d4
  epKC_Unknown, // Qt::Key_Otilde	0x0d5
  epKC_Unknown, // Qt::Key_Odiaeresis	0x0d6
  epKC_NumpadMultiply, // Qt::Key_multiply	0x0d7
  epKC_Unknown, // Qt::Key_Ooblique	0x0d8
  epKC_Unknown, // Qt::Key_Ugrave	0x0d9
  epKC_Unknown, // Qt::Key_Uacute	0x0da
  epKC_Unknown, // Qt::Key_Ucircumflex	0x0db
  epKC_Unknown, // Qt::Key_Udiaeresis	0x0dc
  epKC_Unknown, // Qt::Key_Yacute	0x0dd
  epKC_Unknown, // Qt::Key_THORN	0x0de
  epKC_Unknown, // Qt::Key_ssharp	0x0df

  epKC_Unknown, // e0
  epKC_Unknown, // e1
  epKC_Unknown, // e2
  epKC_Unknown, // e3
  epKC_Unknown, // e4
  epKC_Unknown, // e5
  epKC_Unknown, // e6
  epKC_Unknown, // e7
  epKC_Unknown, // e8
  epKC_Unknown, // e9
  epKC_Unknown, // ea
  epKC_Unknown, // eb
  epKC_Unknown, // ec
  epKC_Unknown, // ed
  epKC_Unknown, // ee
  epKC_Unknown, // ef
  epKC_Unknown, // f0
  epKC_Unknown, // f1
  epKC_Unknown, // f2
  epKC_Unknown, // f3
  epKC_Unknown, // f4
  epKC_Unknown, // f5
  epKC_Unknown, // f6

  epKC_NumpadDivide, // Qt::Key_division	0x0f7

  epKC_Unknown, // f8
  epKC_Unknown, // f9
  epKC_Unknown, // fa
  epKC_Unknown, // fb
  epKC_Unknown, // fc
  epKC_Unknown, // fd
  epKC_Unknown, // fe

  epKC_Unknown, // Qt::Key_ydiaeresis	0x0ff
};

static unsigned char qtKeyToEPKeyHighBit[] =
{
  epKC_Escape, // Qt::Key_Escape	0x01000000
  epKC_Tab, // Qt::Key_Tab	0x01000001
  epKC_Unknown, // Qt::Key_Backtab	0x01000002
  epKC_Backspace, // Qt::Key_Backspace	0x01000003
  epKC_Enter, // Qt::Key_Return	0x01000004
  epKC_Enter, // Qt::Key_Enter	0x01000005	Typically located on the keypad.
  epKC_Insert, // Qt::Key_Insert	0x01000006
  epKC_Delete, // Qt::Key_Delete	0x01000007
  epKC_Pause, // Qt::Key_Pause	0x01000008	The Pause/Break key(Note: Not related to pausing media)
  epKC_PrintScreen, // Qt::Key_Print	0x01000009
  epKC_Unknown, // Qt::Key_SysReq	0x0100000a
  epKC_Unknown, // Qt::Key_Clear	0x0100000b

  epKC_Unknown, // 0c
  epKC_Unknown, // 0d
  epKC_Unknown, // 0e
  epKC_Unknown, // 0f

  epKC_Home, // Qt::Key_Home	0x01000010
  epKC_End, // Qt::Key_End	0x01000011
  epKC_Left, // Qt::Key_Left	0x01000012
  epKC_Up, // Qt::Key_Up	0x01000013
  epKC_Right, // Qt::Key_Right	0x01000014
  epKC_Down, // Qt::Key_Down	0x01000015
  epKC_PageUp, // Qt::Key_PageUp	0x01000016
  epKC_PageDown, // Qt::Key_PageDown	0x01000017

  epKC_Unknown, // 18
  epKC_Unknown, // 19
  epKC_Unknown, // 1a
  epKC_Unknown, // 1b
  epKC_Unknown, // 1c
  epKC_Unknown, // 1d
  epKC_Unknown, // 1e
  epKC_Unknown, // 1f

  epKC_LShift, // Qt::Key_Shift	0x01000020
  epKC_LCtrl, // Qt::Key_Control	0x01000021	On OS X, this corresponds to the Command keys.
  epKC_LWin, // Qt::Key_Meta	0x01000022	On OS X, this corresponds to the Control keys.On Windows keyboards, this key is mapped to the Windows key.
  epKC_LAlt, // Qt::Key_Alt	0x01000023
  epKC_CapsLock, // Qt::Key_CapsLock	0x01000024
  epKC_NumLock, // Qt::Key_NumLock	0x01000025
  epKC_ScrollLock, // Qt::Key_ScrollLock	0x01000026

  epKC_Unknown, // 27
  epKC_Unknown, // 28
  epKC_Unknown, // 29
  epKC_Unknown, // 2a
  epKC_Unknown, // 2b
  epKC_Unknown, // 2c
  epKC_Unknown, // 2d
  epKC_Unknown, // 2e
  epKC_Unknown, // 2f

  epKC_F1, // Qt::Key_F1	0x01000030
  epKC_F2, // Qt::Key_F2	0x01000031
  epKC_F3, // Qt::Key_F3	0x01000032
  epKC_F4, // Qt::Key_F4	0x01000033
  epKC_F5, // Qt::Key_F5	0x01000034
  epKC_F6, // Qt::Key_F6	0x01000035
  epKC_F7, // Qt::Key_F7	0x01000036
  epKC_F8, // Qt::Key_F8	0x01000037
  epKC_F9, // Qt::Key_F9	0x01000038
  epKC_F10, // Qt::Key_F10	0x01000039
  epKC_F11, // Qt::Key_F11	0x0100003a
  epKC_F12, // Qt::Key_F12	0x0100003b
  epKC_Unknown, // Qt::Key_F13	0x0100003c
  epKC_Unknown, // Qt::Key_F14	0x0100003d
  epKC_Unknown, // Qt::Key_F15	0x0100003e
  epKC_Unknown, // Qt::Key_F16	0x0100003f
  epKC_Unknown, // Qt::Key_F17	0x01000040
  epKC_Unknown, // Qt::Key_F18	0x01000041
  epKC_Unknown, // Qt::Key_F19	0x01000042
  epKC_Unknown, // Qt::Key_F20	0x01000043
  epKC_Unknown, // Qt::Key_F21	0x01000044
  epKC_Unknown, // Qt::Key_F22	0x01000045
  epKC_Unknown, // Qt::Key_F23	0x01000046
  epKC_Unknown, // Qt::Key_F24	0x01000047
  epKC_Unknown, // Qt::Key_F25	0x01000048
  epKC_Unknown, // Qt::Key_F26	0x01000049
  epKC_Unknown, // Qt::Key_F27	0x0100004a
  epKC_Unknown, // Qt::Key_F28	0x0100004b
  epKC_Unknown, // Qt::Key_F29	0x0100004c
  epKC_Unknown, // Qt::Key_F30	0x0100004d
  epKC_Unknown, // Qt::Key_F31	0x0100004e
  epKC_Unknown, // Qt::Key_F32	0x0100004f
  epKC_Unknown, // Qt::Key_F33	0x01000050
  epKC_Unknown, // Qt::Key_F34	0x01000051
  epKC_Unknown, // Qt::Key_F35	0x01000052
  epKC_Unknown, // Qt::Key_Super_L	0x01000053
  epKC_Unknown, // Qt::Key_Super_R	0x01000054
  epKC_Menu, // Qt::Key_Menu	0x01000055
  epKC_Unknown, // Qt::Key_Hyper_L	0x01000056
  epKC_Unknown, // Qt::Key_Hyper_R	0x01000057
  epKC_Unknown, // Qt::Key_Help	0x01000058
  epKC_Unknown, // Qt::Key_Direction_L	0x01000059

  epKC_Unknown, // 5a
  epKC_Unknown, // 5b
  epKC_Unknown, // 5c
  epKC_Unknown, // 5d
  epKC_Unknown, // 5e
  epKC_Unknown, // 5f

  epKC_Unknown, // Qt::Key_Direction_R	0x01000060
  epKC_Unknown, // Qt::Key_Back	0x01000061
  epKC_Unknown, // Qt::Key_Forward	0x01000062
  epKC_Unknown, // Qt::Key_Stop	0x01000063
  epKC_Unknown, // Qt::Key_Refresh	0x01000064

  epKC_Unknown, // 65
  epKC_Unknown, // 66
  epKC_Unknown, // 67
  epKC_Unknown, // 68
  epKC_Unknown, // 69
  epKC_Unknown, // 6a
  epKC_Unknown, // 6b
  epKC_Unknown, // 6c
  epKC_Unknown, // 6d
  epKC_Unknown, // 6e
  epKC_Unknown, // 6f

  epKC_Unknown, // Qt::Key_VolumeDown	0x01000070
  epKC_Unknown, // Qt::Key_VolumeMute	0x01000071
  epKC_Unknown, // Qt::Key_VolumeUp	0x01000072
  epKC_Unknown, // Qt::Key_BassBoost	0x01000073
  epKC_Unknown, // Qt::Key_BassUp	0x01000074
  epKC_Unknown, // Qt::Key_BassDown	0x01000075
  epKC_Unknown, // Qt::Key_TrebleUp	0x01000076
  epKC_Unknown, // Qt::Key_TrebleDown	0x01000077

  epKC_Unknown, // 78
  epKC_Unknown, // 79
  epKC_Unknown, // 7a
  epKC_Unknown, // 7b
  epKC_Unknown, // 7c
  epKC_Unknown, // 7d
  epKC_Unknown, // 7e
  epKC_Unknown, // 7f

  epKC_Unknown, // Qt::Key_MediaPlay	0x01000080	A key setting the state of the media player to play
  epKC_Unknown, // Qt::Key_MediaStop	0x01000081	A key setting the state of the media player to stop
  epKC_Unknown, // Qt::Key_MediaPrevious	0x01000082
  epKC_Unknown, // Qt::Key_MediaNext	0x01000083
  epKC_Unknown, // Qt::Key_MediaRecord	0x01000084
  epKC_Unknown, // Qt::Key_MediaPause	0x1000085	A key setting the state of the media player to pause(Note: not the pause/break key)
  epKC_Unknown, // Qt::Key_MediaTogglePlayPause	0x1000086	A key to toggle the play/pause state in the media player(rather than setting an absolute state)

  epKC_Unknown, // 87
  epKC_Unknown, // 88
  epKC_Unknown, // 89
  epKC_Unknown, // 8a
  epKC_Unknown, // 8b
  epKC_Unknown, // 8c
  epKC_Unknown, // 8d
  epKC_Unknown, // 8e
  epKC_Unknown, // 8f

  epKC_Unknown, // Qt::Key_HomePage	0x01000090
  epKC_Unknown, // Qt::Key_Favorites	0x01000091
  epKC_Unknown, // Qt::Key_Search	0x01000092
  epKC_Unknown, // Qt::Key_Standby	0x01000093
  epKC_Unknown, // Qt::Key_OpenUrl	0x01000094

  epKC_Unknown, // 95
  epKC_Unknown, // 96
  epKC_Unknown, // 97
  epKC_Unknown, // 98
  epKC_Unknown, // 99
  epKC_Unknown, // 9a
  epKC_Unknown, // 9b
  epKC_Unknown, // 9c
  epKC_Unknown, // 9d
  epKC_Unknown, // 9e
  epKC_Unknown, // 9f

  epKC_Unknown, // Qt::Key_LaunchMail	0x010000a0
  epKC_Unknown, // Qt::Key_LaunchMedia	0x010000a1
  epKC_Unknown, // Qt::Key_Launch0	0x010000a2	On X11 this key is mapped to "My Computer" (XF86XK_MyComputer)key for legacy reasons.
  epKC_Unknown, // Qt::Key_Launch1	0x010000a3	On X11 this key is mapped to "Calculator" (XF86XK_Calculator)key for legacy reasons.
  epKC_Unknown, // Qt::Key_Launch2	0x010000a4	On X11 this key is mapped to XF86XK_Launch0 key for legacy reasons.
  epKC_Unknown, // Qt::Key_Launch3	0x010000a5	On X11 this key is mapped to XF86XK_Launch1 key for legacy reasons.
  epKC_Unknown, // Qt::Key_Launch4	0x010000a6	On X11 this key is mapped to XF86XK_Launch2 key for legacy reasons.
  epKC_Unknown, // Qt::Key_Launch5	0x010000a7	On X11 this key is mapped to XF86XK_Launch3 key for legacy reasons.
  epKC_Unknown, // Qt::Key_Launch6	0x010000a8	On X11 this key is mapped to XF86XK_Launch4 key for legacy reasons.
  epKC_Unknown, // Qt::Key_Launch7	0x010000a9	On X11 this key is mapped to XF86XK_Launch5 key for legacy reasons.
  epKC_Unknown, // Qt::Key_Launch8	0x010000aa	On X11 this key is mapped to XF86XK_Launch6 key for legacy reasons.
  epKC_Unknown, // Qt::Key_Launch9	0x010000ab	On X11 this key is mapped to XF86XK_Launch7 key for legacy reasons.
  epKC_Unknown, // Qt::Key_LaunchA	0x010000ac	On X11 this key is mapped to XF86XK_Launch8 key for legacy reasons.
  epKC_Unknown, // Qt::Key_LaunchB	0x010000ad	On X11 this key is mapped to XF86XK_Launch9 key for legacy reasons.
  epKC_Unknown, // Qt::Key_LaunchC	0x010000ae	On X11 this key is mapped to XF86XK_LaunchA key for legacy reasons.
  epKC_Unknown, // Qt::Key_LaunchD	0x010000af	On X11 this key is mapped to XF86XK_LaunchB key for legacy reasons.
  epKC_Unknown, // Qt::Key_LaunchE	0x010000b0	On X11 this key is mapped to XF86XK_LaunchC key for legacy reasons.
  epKC_Unknown, // Qt::Key_LaunchF	0x010000b1	On X11 this key is mapped to XF86XK_LaunchD key for legacy reasons.
  epKC_Unknown, // Qt::Key_MonBrightnessUp	0x010000b2
  epKC_Unknown, // Qt::Key_MonBrightnessDown	0x010000b3
  epKC_Unknown, // Qt::Key_KeyboardLightOnOff	0x010000b4
  epKC_Unknown, // Qt::Key_KeyboardBrightnessUp	0x010000b5
  epKC_Unknown, // Qt::Key_KeyboardBrightnessDown	0x010000b6
  epKC_Unknown, // Qt::Key_PowerOff	0x010000b7
  epKC_Unknown, // Qt::Key_WakeUp	0x010000b8
  epKC_Unknown, // Qt::Key_Eject	0x010000b9
  epKC_Unknown, // Qt::Key_ScreenSaver	0x010000ba
  epKC_Unknown, // Qt::Key_WWW	0x010000bb
  epKC_Unknown, // Qt::Key_Memo	0x010000bc
  epKC_Unknown, // Qt::Key_LightBulb	0x010000bd
  epKC_Unknown, // Qt::Key_Shop	0x010000be
  epKC_Unknown, // Qt::Key_History	0x010000bf
  epKC_Unknown, // Qt::Key_AddFavorite	0x010000c0
  epKC_Unknown, // Qt::Key_HotLinks	0x010000c1
  epKC_Unknown, // Qt::Key_BrightnessAdjust	0x010000c2
  epKC_Unknown, // Qt::Key_Finance	0x010000c3
  epKC_Unknown, // Qt::Key_Community	0x010000c4
  epKC_Unknown, // Qt::Key_AudioRewind	0x010000c5
  epKC_Unknown, // Qt::Key_BackForward	0x010000c6
  epKC_Unknown, // Qt::Key_ApplicationLeft	0x010000c7
  epKC_Unknown, // Qt::Key_ApplicationRight	0x010000c8
  epKC_Unknown, // Qt::Key_Book	0x010000c9
  epKC_Unknown, // Qt::Key_CD	0x010000ca
  epKC_Unknown, // Qt::Key_Calculator	0x010000cb	On X11 this key is not mapped for legacy reasons.Use Qt::Key_Launch1 instead.
  epKC_Unknown, // Qt::Key_ToDoList	0x010000cc
  epKC_Unknown, // Qt::Key_ClearGrab	0x010000cd
  epKC_Unknown, // Qt::Key_Close	0x010000ce
  epKC_Unknown, // Qt::Key_Copy	0x010000cf
  epKC_Unknown, // Qt::Key_Cut	0x010000d0
  epKC_Unknown, // Qt::Key_Display	0x010000d1
  epKC_Unknown, // Qt::Key_DOS	0x010000d2
  epKC_Unknown, // Qt::Key_Documents	0x010000d3
  epKC_Unknown, // Qt::Key_Excel	0x010000d4
  epKC_Unknown, // Qt::Key_Explorer	0x010000d5
  epKC_Unknown, // Qt::Key_Game	0x010000d6
  epKC_Unknown, // Qt::Key_Go	0x010000d7
  epKC_Unknown, // Qt::Key_iTouch	0x010000d8
  epKC_Unknown, // Qt::Key_LogOff	0x010000d9
  epKC_Unknown, // Qt::Key_Market	0x010000da
  epKC_Unknown, // Qt::Key_Meeting	0x010000db
  epKC_Unknown, // Qt::Key_MenuKB	0x010000dc
  epKC_Unknown, // Qt::Key_MenuPB	0x010000dd
  epKC_Unknown, // Qt::Key_MySites	0x010000de
  epKC_Unknown, // Qt::Key_News	0x010000df
  epKC_Unknown, // Qt::Key_OfficeHome	0x010000e0
  epKC_Unknown, // Qt::Key_Option	0x010000e1
  epKC_Unknown, // Qt::Key_Paste	0x010000e2
  epKC_Unknown, // Qt::Key_Phone	0x010000e3
  epKC_Unknown, // Qt::Key_Calendar	0x010000e4
  epKC_Unknown, // Qt::Key_Reply	0x010000e5
  epKC_Unknown, // Qt::Key_Reload	0x010000e6
  epKC_Unknown, // Qt::Key_RotateWindows	0x010000e7
  epKC_Unknown, // Qt::Key_RotationPB	0x010000e8
  epKC_Unknown, // Qt::Key_RotationKB	0x010000e9
  epKC_Unknown, // Qt::Key_Save	0x010000ea
  epKC_Unknown, // Qt::Key_Send	0x010000eb
  epKC_Unknown, // Qt::Key_Spell	0x010000ec
  epKC_Unknown, // Qt::Key_SplitScreen	0x010000ed
  epKC_Unknown, // Qt::Key_Support	0x010000ee
  epKC_Unknown, // Qt::Key_TaskPane	0x010000ef
  epKC_Unknown, // Qt::Key_Terminal	0x010000f0
  epKC_Unknown, // Qt::Key_Tools	0x010000f1
  epKC_Unknown, // Qt::Key_Travel	0x010000f2
  epKC_Unknown, // Qt::Key_Video	0x010000f3
  epKC_Unknown, // Qt::Key_Word	0x010000f4
  epKC_Unknown, // Qt::Key_Xfer	0x010000f5
  epKC_Unknown, // Qt::Key_ZoomIn	0x010000f6
  epKC_Unknown, // Qt::Key_ZoomOut	0x010000f7
  epKC_Unknown, // Qt::Key_Away	0x010000f8
  epKC_Unknown, // Qt::Key_Messenger	0x010000f9
  epKC_Unknown, // Qt::Key_WebCam	0x010000fa
  epKC_Unknown, // Qt::Key_MailForward	0x010000fb
  epKC_Unknown, // Qt::Key_Pictures	0x010000fc
  epKC_Unknown, // Qt::Key_Music	0x010000fd
  epKC_Unknown, // Qt::Key_Battery	0x010000fe
  epKC_Unknown, // Qt::Key_Bluetooth	0x010000ff
  epKC_Unknown, // Qt::Key_WLAN	0x01000100
  epKC_Unknown, // Qt::Key_UWB	0x01000101
  epKC_Unknown, // Qt::Key_AudioForward	0x01000102
  epKC_Unknown, // Qt::Key_AudioRepeat	0x01000103
  epKC_Unknown, // Qt::Key_AudioRandomPlay	0x01000104
  epKC_Unknown, // Qt::Key_Subtitle	0x01000105
  epKC_Unknown, // Qt::Key_AudioCycleTrack	0x01000106
  epKC_Unknown, // Qt::Key_Time	0x01000107
  epKC_Unknown, // Qt::Key_Hibernate	0x01000108
  epKC_Unknown, // Qt::Key_View	0x01000109
  epKC_Unknown, // Qt::Key_TopMenu	0x0100010a
  epKC_Unknown, // Qt::Key_PowerDown	0x0100010b
  epKC_Unknown, // Qt::Key_Suspend	0x0100010c
  epKC_Unknown, // Qt::Key_ContrastAdjust	0x0100010d
  epKC_Unknown, // Qt::Key_LaunchG	0x0100010e	On X11 this key is mapped to XF86XK_LaunchE key for legacy reasons.
  epKC_Unknown, // Qt::Key_LaunchH	0x0100010f	On X11 this key is mapped to XF86XK_LaunchF key for legacy reasons.
  epKC_Unknown, // Qt::Key_TouchpadToggle	0x01000110
  epKC_Unknown, // Qt::Key_TouchpadOn	0x01000111
  epKC_Unknown, // Qt::Key_TouchpadOff	0x01000112
  epKC_Unknown, // Qt::Key_MicMute	0x01000113
  epKC_Unknown, // Qt::Key_Red	0x01000114
  epKC_Unknown, // Qt::Key_Green	0x01000115
  epKC_Unknown, // Qt::Key_Yellow	0x01000116
  epKC_Unknown, // Qt::Key_Blue	0x01000117
  epKC_Unknown, // Qt::Key_ChannelUp	0x01000118
  epKC_Unknown, // Qt::Key_ChannelDown	0x01000119
};

epKeyCode qtKeyToEPKey(Qt::Key qk)
{
  if ((qk & ~0xFF) == 0)
    return (epKeyCode)qtKeyToEPKeyLower[qk - 0x20];
  else if ((qk & 0xFFFFFF00) == 0x01000000)
    return (epKeyCode)qtKeyToEPKeyHighBit[qk & 0xFFF];
  else
  {
//    epKC_RAlt, // Qt::Key_AltGr	0x01001103	On Windows, when the KeyDown event for this key is sent, the Ctrl+Alt modifiers are also set.
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

    return epKC_Unknown;
  }
}

#else
EPEMPTYFILE
#endif
