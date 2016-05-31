#include "driver.h"

#if EPINPUT_DRIVER == EPDRIVER_QT

#include "input_internal.h"

#include <Qt>

using namespace ep;

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
  (unsigned char)KeyCode::Space, // Qt::Key_Space	0x20
  (unsigned char)KeyCode::Unknown, // Qt::Key_Exclam	0x21
  (unsigned char)KeyCode::Unknown, // Qt::Key_QuoteDbl	0x22
  (unsigned char)KeyCode::Unknown, // Qt::Key_NumberSign	0x23
  (unsigned char)KeyCode::Unknown, // Qt::Key_Dollar	0x24
  (unsigned char)KeyCode::Unknown, // Qt::Key_Percent	0x25
  (unsigned char)KeyCode::Unknown, // Qt::Key_Ampersand	0x26
  (unsigned char)KeyCode::Apostrophe, // Qt::Key_Apostrophe	0x27
  (unsigned char)KeyCode::Unknown, // Qt::Key_ParenLeft	0x28
  (unsigned char)KeyCode::Unknown, // Qt::Key_ParenRight	0x29
  (unsigned char)KeyCode::NumpadMultiply, // Qt::Key_Asterisk	0x2a
  (unsigned char)KeyCode::NumpadPlus, // Qt::Key_Plus	0x2b
  (unsigned char)KeyCode::Comma, // Qt::Key_Comma	0x2c
  (unsigned char)KeyCode::NumpadMinus, // Qt::Key_Minus	0x2d
  (unsigned char)KeyCode::Period, // Qt::Key_Period	0x2e
  (unsigned char)KeyCode::ForwardSlash, // Qt::Key_Slash	0x2f
  (unsigned char)KeyCode::_0, // Qt::Key_0	0x30
  (unsigned char)KeyCode::_1, // Qt::Key_1	0x31
  (unsigned char)KeyCode::_2, // Qt::Key_2	0x32
  (unsigned char)KeyCode::_3, // Qt::Key_3	0x33
  (unsigned char)KeyCode::_4, // Qt::Key_4	0x34
  (unsigned char)KeyCode::_5, // Qt::Key_5	0x35
  (unsigned char)KeyCode::_6, // Qt::Key_6	0x36
  (unsigned char)KeyCode::_7, // Qt::Key_7	0x37
  (unsigned char)KeyCode::_8, // Qt::Key_8	0x38
  (unsigned char)KeyCode::_9, // Qt::Key_9	0x39
  (unsigned char)KeyCode::Unknown, // Qt::Key_Colon	0x3a
  (unsigned char)KeyCode::Semicolon, // Qt::Key_Semicolon	0x3b
  (unsigned char)KeyCode::Unknown, // Qt::Key_Less	0x3c
  (unsigned char)KeyCode::Equals, // Qt::Key_Equal	0x3d
  (unsigned char)KeyCode::Unknown, // Qt::Key_Greater	0x3e
  (unsigned char)KeyCode::Unknown, // Qt::Key_Question	0x3f
  (unsigned char)KeyCode::Unknown, // Qt::Key_At	0x40
  (unsigned char)KeyCode::A, // Qt::Key_A	0x41
  (unsigned char)KeyCode::B, // Qt::Key_B	0x42
  (unsigned char)KeyCode::C, // Qt::Key_C	0x43
  (unsigned char)KeyCode::D, // Qt::Key_D	0x44
  (unsigned char)KeyCode::E, // Qt::Key_E	0x45
  (unsigned char)KeyCode::F, // Qt::Key_F	0x46
  (unsigned char)KeyCode::G, // Qt::Key_G	0x47
  (unsigned char)KeyCode::H, // Qt::Key_H	0x48
  (unsigned char)KeyCode::I, // Qt::Key_I	0x49
  (unsigned char)KeyCode::J, // Qt::Key_J	0x4a
  (unsigned char)KeyCode::K, // Qt::Key_K	0x4b
  (unsigned char)KeyCode::L, // Qt::Key_L	0x4c
  (unsigned char)KeyCode::M, // Qt::Key_M	0x4d
  (unsigned char)KeyCode::N, // Qt::Key_N	0x4e
  (unsigned char)KeyCode::O, // Qt::Key_O	0x4f
  (unsigned char)KeyCode::P, // Qt::Key_P	0x50
  (unsigned char)KeyCode::Q, // Qt::Key_Q	0x51
  (unsigned char)KeyCode::R, // Qt::Key_R	0x52
  (unsigned char)KeyCode::S, // Qt::Key_S	0x53
  (unsigned char)KeyCode::T, // Qt::Key_T	0x54
  (unsigned char)KeyCode::U, // Qt::Key_U	0x55
  (unsigned char)KeyCode::V, // Qt::Key_V	0x56
  (unsigned char)KeyCode::W, // Qt::Key_W	0x57
  (unsigned char)KeyCode::X, // Qt::Key_X	0x58
  (unsigned char)KeyCode::Y, // Qt::Key_Y	0x59
  (unsigned char)KeyCode::Z, // Qt::Key_Z	0x5a
  (unsigned char)KeyCode::LeftBracket, // Qt::Key_BracketLeft	0x5b
  (unsigned char)KeyCode::BackSlash, // Qt::Key_Backslash	0x5c
  (unsigned char)KeyCode::RightBracket, // Qt::Key_BracketRight	0x5d
  (unsigned char)KeyCode::Unknown, // Qt::Key_AsciiCircum	0x5e
  (unsigned char)KeyCode::Unknown, // Qt::Key_Underscore	0x5f
  (unsigned char)KeyCode::Unknown, // Qt::Key_QuoteLeft	0x60

  (unsigned char)KeyCode::Unknown, // 61
  (unsigned char)KeyCode::Unknown, // 62
  (unsigned char)KeyCode::Unknown, // 63
  (unsigned char)KeyCode::Unknown, // 64
  (unsigned char)KeyCode::Unknown, // 65
  (unsigned char)KeyCode::Unknown, // 66
  (unsigned char)KeyCode::Unknown, // 67
  (unsigned char)KeyCode::Unknown, // 68
  (unsigned char)KeyCode::Unknown, // 69
  (unsigned char)KeyCode::Unknown, // 6a
  (unsigned char)KeyCode::Unknown, // 6b
  (unsigned char)KeyCode::Unknown, // 6c
  (unsigned char)KeyCode::Unknown, // 6d
  (unsigned char)KeyCode::Unknown, // 6e
  (unsigned char)KeyCode::Unknown, // 6f
  (unsigned char)KeyCode::Unknown, // 70
  (unsigned char)KeyCode::Unknown, // 71
  (unsigned char)KeyCode::Unknown, // 72
  (unsigned char)KeyCode::Unknown, // 73
  (unsigned char)KeyCode::Unknown, // 74
  (unsigned char)KeyCode::Unknown, // 75
  (unsigned char)KeyCode::Unknown, // 76
  (unsigned char)KeyCode::Unknown, // 77
  (unsigned char)KeyCode::Unknown, // 78
  (unsigned char)KeyCode::Unknown, // 79
  (unsigned char)KeyCode::Unknown, // 7a

  (unsigned char)KeyCode::Unknown, // Qt::Key_BraceLeft	0x7b
  (unsigned char)KeyCode::Unknown, // Qt::Key_Bar	0x7c
  (unsigned char)KeyCode::Unknown, // Qt::Key_BraceRight	0x7d
  (unsigned char)KeyCode::Unknown, // Qt::Key_AsciiTilde	0x7e

  (unsigned char)KeyCode::Unknown, // 7f
  (unsigned char)KeyCode::Unknown, // 80
  (unsigned char)KeyCode::Unknown, // 81
  (unsigned char)KeyCode::Unknown, // 82
  (unsigned char)KeyCode::Unknown, // 83
  (unsigned char)KeyCode::Unknown, // 84
  (unsigned char)KeyCode::Unknown, // 85
  (unsigned char)KeyCode::Unknown, // 86
  (unsigned char)KeyCode::Unknown, // 87
  (unsigned char)KeyCode::Unknown, // 88
  (unsigned char)KeyCode::Unknown, // 89
  (unsigned char)KeyCode::Unknown, // 8a
  (unsigned char)KeyCode::Unknown, // 8b
  (unsigned char)KeyCode::Unknown, // 8c
  (unsigned char)KeyCode::Unknown, // 8d
  (unsigned char)KeyCode::Unknown, // 8e
  (unsigned char)KeyCode::Unknown, // 8f

  (unsigned char)KeyCode::Unknown, // Qt::Key_nobreakspace	0x0a0
  (unsigned char)KeyCode::Unknown, // Qt::Key_exclamdown	0x0a1
  (unsigned char)KeyCode::Unknown, // Qt::Key_cent	0x0a2
  (unsigned char)KeyCode::Unknown, // Qt::Key_sterling	0x0a3
  (unsigned char)KeyCode::Unknown, // Qt::Key_currency	0x0a4
  (unsigned char)KeyCode::Unknown, // Qt::Key_yen	0x0a5
  (unsigned char)KeyCode::Unknown, // Qt::Key_brokenbar	0x0a6
  (unsigned char)KeyCode::Unknown, // Qt::Key_section	0x0a7
  (unsigned char)KeyCode::Unknown, // Qt::Key_diaeresis	0x0a8
  (unsigned char)KeyCode::Unknown, // Qt::Key_copyright	0x0a9
  (unsigned char)KeyCode::Unknown, // Qt::Key_ordfeminine	0x0aa
  (unsigned char)KeyCode::Unknown, // Qt::Key_guillemotleft	0x0ab
  (unsigned char)KeyCode::Unknown, // Qt::Key_notsign	0x0ac
  (unsigned char)KeyCode::Hyphen, // Qt::Key_hyphen	0x0ad
  (unsigned char)KeyCode::Unknown, // Qt::Key_registered	0x0ae
  (unsigned char)KeyCode::Unknown, // Qt::Key_macron	0x0af
  (unsigned char)KeyCode::Unknown, // Qt::Key_degree	0x0b0
  (unsigned char)KeyCode::Unknown, // Qt::Key_plusminus	0x0b1
  (unsigned char)KeyCode::Unknown, // Qt::Key_twosuperior	0x0b2
  (unsigned char)KeyCode::Unknown, // Qt::Key_threesuperior	0x0b3
  (unsigned char)KeyCode::Unknown, // Qt::Key_acute	0x0b4
  (unsigned char)KeyCode::Unknown, // Qt::Key_mu	0x0b5
  (unsigned char)KeyCode::Unknown, // Qt::Key_paragraph	0x0b6
  (unsigned char)KeyCode::Unknown, // Qt::Key_periodcentered	0x0b7
  (unsigned char)KeyCode::Unknown, // Qt::Key_cedilla	0x0b8
  (unsigned char)KeyCode::Unknown, // Qt::Key_onesuperior	0x0b9
  (unsigned char)KeyCode::Unknown, // Qt::Key_masculine	0x0ba
  (unsigned char)KeyCode::Unknown, // Qt::Key_guillemotright	0x0bb
  (unsigned char)KeyCode::Unknown, // Qt::Key_onequarter	0x0bc
  (unsigned char)KeyCode::Unknown, // Qt::Key_onehalf	0x0bd
  (unsigned char)KeyCode::Unknown, // Qt::Key_threequarters	0x0be
  (unsigned char)KeyCode::Unknown, // Qt::Key_questiondown	0x0bf
  (unsigned char)KeyCode::Unknown, // Qt::Key_Agrave	0x0c0
  (unsigned char)KeyCode::Unknown, // Qt::Key_Aacute	0x0c1
  (unsigned char)KeyCode::Unknown, // Qt::Key_Acircumflex	0x0c2
  (unsigned char)KeyCode::Unknown, // Qt::Key_Atilde	0x0c3
  (unsigned char)KeyCode::Unknown, // Qt::Key_Adiaeresis	0x0c4
  (unsigned char)KeyCode::Unknown, // Qt::Key_Aring	0x0c5
  (unsigned char)KeyCode::Unknown, // Qt::Key_AE	0x0c6
  (unsigned char)KeyCode::Unknown, // Qt::Key_Ccedilla	0x0c7
  (unsigned char)KeyCode::Unknown, // Qt::Key_Egrave	0x0c8
  (unsigned char)KeyCode::Unknown, // Qt::Key_Eacute	0x0c9
  (unsigned char)KeyCode::Unknown, // Qt::Key_Ecircumflex	0x0ca
  (unsigned char)KeyCode::Unknown, // Qt::Key_Ediaeresis	0x0cb
  (unsigned char)KeyCode::Unknown, // Qt::Key_Igrave	0x0cc
  (unsigned char)KeyCode::Unknown, // Qt::Key_Iacute	0x0cd
  (unsigned char)KeyCode::Unknown, // Qt::Key_Icircumflex	0x0ce
  (unsigned char)KeyCode::Unknown, // Qt::Key_Idiaeresis	0x0cf
  (unsigned char)KeyCode::Unknown, // Qt::Key_ETH	0x0d0
  (unsigned char)KeyCode::Unknown, // Qt::Key_Ntilde	0x0d1
  (unsigned char)KeyCode::Unknown, // Qt::Key_Ograve	0x0d2
  (unsigned char)KeyCode::Unknown, // Qt::Key_Oacute	0x0d3
  (unsigned char)KeyCode::Unknown, // Qt::Key_Ocircumflex	0x0d4
  (unsigned char)KeyCode::Unknown, // Qt::Key_Otilde	0x0d5
  (unsigned char)KeyCode::Unknown, // Qt::Key_Odiaeresis	0x0d6
  (unsigned char)KeyCode::NumpadMultiply, // Qt::Key_multiply	0x0d7
  (unsigned char)KeyCode::Unknown, // Qt::Key_Ooblique	0x0d8
  (unsigned char)KeyCode::Unknown, // Qt::Key_Ugrave	0x0d9
  (unsigned char)KeyCode::Unknown, // Qt::Key_Uacute	0x0da
  (unsigned char)KeyCode::Unknown, // Qt::Key_Ucircumflex	0x0db
  (unsigned char)KeyCode::Unknown, // Qt::Key_Udiaeresis	0x0dc
  (unsigned char)KeyCode::Unknown, // Qt::Key_Yacute	0x0dd
  (unsigned char)KeyCode::Unknown, // Qt::Key_THORN	0x0de
  (unsigned char)KeyCode::Unknown, // Qt::Key_ssharp	0x0df

  (unsigned char)KeyCode::Unknown, // e0
  (unsigned char)KeyCode::Unknown, // e1
  (unsigned char)KeyCode::Unknown, // e2
  (unsigned char)KeyCode::Unknown, // e3
  (unsigned char)KeyCode::Unknown, // e4
  (unsigned char)KeyCode::Unknown, // e5
  (unsigned char)KeyCode::Unknown, // e6
  (unsigned char)KeyCode::Unknown, // e7
  (unsigned char)KeyCode::Unknown, // e8
  (unsigned char)KeyCode::Unknown, // e9
  (unsigned char)KeyCode::Unknown, // ea
  (unsigned char)KeyCode::Unknown, // eb
  (unsigned char)KeyCode::Unknown, // ec
  (unsigned char)KeyCode::Unknown, // ed
  (unsigned char)KeyCode::Unknown, // ee
  (unsigned char)KeyCode::Unknown, // ef
  (unsigned char)KeyCode::Unknown, // f0
  (unsigned char)KeyCode::Unknown, // f1
  (unsigned char)KeyCode::Unknown, // f2
  (unsigned char)KeyCode::Unknown, // f3
  (unsigned char)KeyCode::Unknown, // f4
  (unsigned char)KeyCode::Unknown, // f5
  (unsigned char)KeyCode::Unknown, // f6

  (unsigned char)KeyCode::NumpadDivide, // Qt::Key_division	0x0f7

  (unsigned char)KeyCode::Unknown, // f8
  (unsigned char)KeyCode::Unknown, // f9
  (unsigned char)KeyCode::Unknown, // fa
  (unsigned char)KeyCode::Unknown, // fb
  (unsigned char)KeyCode::Unknown, // fc
  (unsigned char)KeyCode::Unknown, // fd
  (unsigned char)KeyCode::Unknown, // fe

  (unsigned char)KeyCode::Unknown, // Qt::Key_ydiaeresis	0x0ff
};

static unsigned char qtKeyToEPKeyHighBit[] =
{
  (unsigned char)KeyCode::Escape, // Qt::Key_Escape	0x01000000
  (unsigned char)KeyCode::Tab, // Qt::Key_Tab	0x01000001
  (unsigned char)KeyCode::Unknown, // Qt::Key_Backtab	0x01000002
  (unsigned char)KeyCode::Backspace, // Qt::Key_Backspace	0x01000003
  (unsigned char)KeyCode::Enter, // Qt::Key_Return	0x01000004
  (unsigned char)KeyCode::Enter, // Qt::Key_Enter	0x01000005	Typically located on the keypad.
  (unsigned char)KeyCode::Insert, // Qt::Key_Insert	0x01000006
  (unsigned char)KeyCode::Delete, // Qt::Key_Delete	0x01000007
  (unsigned char)KeyCode::Pause, // Qt::Key_Pause	0x01000008	The Pause/Break key(Note: Not related to pausing media)
  (unsigned char)KeyCode::PrintScreen, // Qt::Key_Print	0x01000009
  (unsigned char)KeyCode::Unknown, // Qt::Key_SysReq	0x0100000a
  (unsigned char)KeyCode::Unknown, // Qt::Key_Clear	0x0100000b

  (unsigned char)KeyCode::Unknown, // 0c
  (unsigned char)KeyCode::Unknown, // 0d
  (unsigned char)KeyCode::Unknown, // 0e
  (unsigned char)KeyCode::Unknown, // 0f

  (unsigned char)KeyCode::Home, // Qt::Key_Home	0x01000010
  (unsigned char)KeyCode::End, // Qt::Key_End	0x01000011
  (unsigned char)KeyCode::Left, // Qt::Key_Left	0x01000012
  (unsigned char)KeyCode::Up, // Qt::Key_Up	0x01000013
  (unsigned char)KeyCode::Right, // Qt::Key_Right	0x01000014
  (unsigned char)KeyCode::Down, // Qt::Key_Down	0x01000015
  (unsigned char)KeyCode::PageUp, // Qt::Key_PageUp	0x01000016
  (unsigned char)KeyCode::PageDown, // Qt::Key_PageDown	0x01000017

  (unsigned char)KeyCode::Unknown, // 18
  (unsigned char)KeyCode::Unknown, // 19
  (unsigned char)KeyCode::Unknown, // 1a
  (unsigned char)KeyCode::Unknown, // 1b
  (unsigned char)KeyCode::Unknown, // 1c
  (unsigned char)KeyCode::Unknown, // 1d
  (unsigned char)KeyCode::Unknown, // 1e
  (unsigned char)KeyCode::Unknown, // 1f

  (unsigned char)KeyCode::LShift, // Qt::Key_Shift	0x01000020
  (unsigned char)KeyCode::LCtrl, // Qt::Key_Control	0x01000021	On OS X, this corresponds to the Command keys.
  (unsigned char)KeyCode::LWin, // Qt::Key_Meta	0x01000022	On OS X, this corresponds to the Control keys.On Windows keyboards, this key is mapped to the Windows key.
  (unsigned char)KeyCode::LAlt, // Qt::Key_Alt	0x01000023
  (unsigned char)KeyCode::CapsLock, // Qt::Key_CapsLock	0x01000024
  (unsigned char)KeyCode::NumLock, // Qt::Key_NumLock	0x01000025
  (unsigned char)KeyCode::ScrollLock, // Qt::Key_ScrollLock	0x01000026

  (unsigned char)KeyCode::Unknown, // 27
  (unsigned char)KeyCode::Unknown, // 28
  (unsigned char)KeyCode::Unknown, // 29
  (unsigned char)KeyCode::Unknown, // 2a
  (unsigned char)KeyCode::Unknown, // 2b
  (unsigned char)KeyCode::Unknown, // 2c
  (unsigned char)KeyCode::Unknown, // 2d
  (unsigned char)KeyCode::Unknown, // 2e
  (unsigned char)KeyCode::Unknown, // 2f

  (unsigned char)KeyCode::F1, // Qt::Key_F1	0x01000030
  (unsigned char)KeyCode::F2, // Qt::Key_F2	0x01000031
  (unsigned char)KeyCode::F3, // Qt::Key_F3	0x01000032
  (unsigned char)KeyCode::F4, // Qt::Key_F4	0x01000033
  (unsigned char)KeyCode::F5, // Qt::Key_F5	0x01000034
  (unsigned char)KeyCode::F6, // Qt::Key_F6	0x01000035
  (unsigned char)KeyCode::F7, // Qt::Key_F7	0x01000036
  (unsigned char)KeyCode::F8, // Qt::Key_F8	0x01000037
  (unsigned char)KeyCode::F9, // Qt::Key_F9	0x01000038
  (unsigned char)KeyCode::F10, // Qt::Key_F10	0x01000039
  (unsigned char)KeyCode::F11, // Qt::Key_F11	0x0100003a
  (unsigned char)KeyCode::F12, // Qt::Key_F12	0x0100003b
  (unsigned char)KeyCode::Unknown, // Qt::Key_F13	0x0100003c
  (unsigned char)KeyCode::Unknown, // Qt::Key_F14	0x0100003d
  (unsigned char)KeyCode::Unknown, // Qt::Key_F15	0x0100003e
  (unsigned char)KeyCode::Unknown, // Qt::Key_F16	0x0100003f
  (unsigned char)KeyCode::Unknown, // Qt::Key_F17	0x01000040
  (unsigned char)KeyCode::Unknown, // Qt::Key_F18	0x01000041
  (unsigned char)KeyCode::Unknown, // Qt::Key_F19	0x01000042
  (unsigned char)KeyCode::Unknown, // Qt::Key_F20	0x01000043
  (unsigned char)KeyCode::Unknown, // Qt::Key_F21	0x01000044
  (unsigned char)KeyCode::Unknown, // Qt::Key_F22	0x01000045
  (unsigned char)KeyCode::Unknown, // Qt::Key_F23	0x01000046
  (unsigned char)KeyCode::Unknown, // Qt::Key_F24	0x01000047
  (unsigned char)KeyCode::Unknown, // Qt::Key_F25	0x01000048
  (unsigned char)KeyCode::Unknown, // Qt::Key_F26	0x01000049
  (unsigned char)KeyCode::Unknown, // Qt::Key_F27	0x0100004a
  (unsigned char)KeyCode::Unknown, // Qt::Key_F28	0x0100004b
  (unsigned char)KeyCode::Unknown, // Qt::Key_F29	0x0100004c
  (unsigned char)KeyCode::Unknown, // Qt::Key_F30	0x0100004d
  (unsigned char)KeyCode::Unknown, // Qt::Key_F31	0x0100004e
  (unsigned char)KeyCode::Unknown, // Qt::Key_F32	0x0100004f
  (unsigned char)KeyCode::Unknown, // Qt::Key_F33	0x01000050
  (unsigned char)KeyCode::Unknown, // Qt::Key_F34	0x01000051
  (unsigned char)KeyCode::Unknown, // Qt::Key_F35	0x01000052
  (unsigned char)KeyCode::Unknown, // Qt::Key_Super_L	0x01000053
  (unsigned char)KeyCode::Unknown, // Qt::Key_Super_R	0x01000054
  (unsigned char)KeyCode::Menu, // Qt::Key_Menu	0x01000055
  (unsigned char)KeyCode::Unknown, // Qt::Key_Hyper_L	0x01000056
  (unsigned char)KeyCode::Unknown, // Qt::Key_Hyper_R	0x01000057
  (unsigned char)KeyCode::Unknown, // Qt::Key_Help	0x01000058
  (unsigned char)KeyCode::Unknown, // Qt::Key_Direction_L	0x01000059

  (unsigned char)KeyCode::Unknown, // 5a
  (unsigned char)KeyCode::Unknown, // 5b
  (unsigned char)KeyCode::Unknown, // 5c
  (unsigned char)KeyCode::Unknown, // 5d
  (unsigned char)KeyCode::Unknown, // 5e
  (unsigned char)KeyCode::Unknown, // 5f

  (unsigned char)KeyCode::Unknown, // Qt::Key_Direction_R	0x01000060
  (unsigned char)KeyCode::Unknown, // Qt::Key_Back	0x01000061
  (unsigned char)KeyCode::Unknown, // Qt::Key_Forward	0x01000062
  (unsigned char)KeyCode::Unknown, // Qt::Key_Stop	0x01000063
  (unsigned char)KeyCode::Unknown, // Qt::Key_Refresh	0x01000064

  (unsigned char)KeyCode::Unknown, // 65
  (unsigned char)KeyCode::Unknown, // 66
  (unsigned char)KeyCode::Unknown, // 67
  (unsigned char)KeyCode::Unknown, // 68
  (unsigned char)KeyCode::Unknown, // 69
  (unsigned char)KeyCode::Unknown, // 6a
  (unsigned char)KeyCode::Unknown, // 6b
  (unsigned char)KeyCode::Unknown, // 6c
  (unsigned char)KeyCode::Unknown, // 6d
  (unsigned char)KeyCode::Unknown, // 6e
  (unsigned char)KeyCode::Unknown, // 6f

  (unsigned char)KeyCode::Unknown, // Qt::Key_VolumeDown	0x01000070
  (unsigned char)KeyCode::Unknown, // Qt::Key_VolumeMute	0x01000071
  (unsigned char)KeyCode::Unknown, // Qt::Key_VolumeUp	0x01000072
  (unsigned char)KeyCode::Unknown, // Qt::Key_BassBoost	0x01000073
  (unsigned char)KeyCode::Unknown, // Qt::Key_BassUp	0x01000074
  (unsigned char)KeyCode::Unknown, // Qt::Key_BassDown	0x01000075
  (unsigned char)KeyCode::Unknown, // Qt::Key_TrebleUp	0x01000076
  (unsigned char)KeyCode::Unknown, // Qt::Key_TrebleDown	0x01000077

  (unsigned char)KeyCode::Unknown, // 78
  (unsigned char)KeyCode::Unknown, // 79
  (unsigned char)KeyCode::Unknown, // 7a
  (unsigned char)KeyCode::Unknown, // 7b
  (unsigned char)KeyCode::Unknown, // 7c
  (unsigned char)KeyCode::Unknown, // 7d
  (unsigned char)KeyCode::Unknown, // 7e
  (unsigned char)KeyCode::Unknown, // 7f

  (unsigned char)KeyCode::Unknown, // Qt::Key_MediaPlay	0x01000080	A key setting the state of the media player to play
  (unsigned char)KeyCode::Unknown, // Qt::Key_MediaStop	0x01000081	A key setting the state of the media player to stop
  (unsigned char)KeyCode::Unknown, // Qt::Key_MediaPrevious	0x01000082
  (unsigned char)KeyCode::Unknown, // Qt::Key_MediaNext	0x01000083
  (unsigned char)KeyCode::Unknown, // Qt::Key_MediaRecord	0x01000084
  (unsigned char)KeyCode::Unknown, // Qt::Key_MediaPause	0x1000085	A key setting the state of the media player to pause(Note: not the pause/break key)
  (unsigned char)KeyCode::Unknown, // Qt::Key_MediaTogglePlayPause	0x1000086	A key to toggle the play/pause state in the media player(rather than setting an absolute state)

  (unsigned char)KeyCode::Unknown, // 87
  (unsigned char)KeyCode::Unknown, // 88
  (unsigned char)KeyCode::Unknown, // 89
  (unsigned char)KeyCode::Unknown, // 8a
  (unsigned char)KeyCode::Unknown, // 8b
  (unsigned char)KeyCode::Unknown, // 8c
  (unsigned char)KeyCode::Unknown, // 8d
  (unsigned char)KeyCode::Unknown, // 8e
  (unsigned char)KeyCode::Unknown, // 8f

  (unsigned char)KeyCode::Unknown, // Qt::Key_HomePage	0x01000090
  (unsigned char)KeyCode::Unknown, // Qt::Key_Favorites	0x01000091
  (unsigned char)KeyCode::Unknown, // Qt::Key_Search	0x01000092
  (unsigned char)KeyCode::Unknown, // Qt::Key_Standby	0x01000093
  (unsigned char)KeyCode::Unknown, // Qt::Key_OpenUrl	0x01000094

  (unsigned char)KeyCode::Unknown, // 95
  (unsigned char)KeyCode::Unknown, // 96
  (unsigned char)KeyCode::Unknown, // 97
  (unsigned char)KeyCode::Unknown, // 98
  (unsigned char)KeyCode::Unknown, // 99
  (unsigned char)KeyCode::Unknown, // 9a
  (unsigned char)KeyCode::Unknown, // 9b
  (unsigned char)KeyCode::Unknown, // 9c
  (unsigned char)KeyCode::Unknown, // 9d
  (unsigned char)KeyCode::Unknown, // 9e
  (unsigned char)KeyCode::Unknown, // 9f

  (unsigned char)KeyCode::Unknown, // Qt::Key_LaunchMail	0x010000a0
  (unsigned char)KeyCode::Unknown, // Qt::Key_LaunchMedia	0x010000a1
  (unsigned char)KeyCode::Unknown, // Qt::Key_Launch0	0x010000a2	On X11 this key is mapped to "My Computer" (XF86XK_MyComputer)key for legacy reasons.
  (unsigned char)KeyCode::Unknown, // Qt::Key_Launch1	0x010000a3	On X11 this key is mapped to "Calculator" (XF86XK_Calculator)key for legacy reasons.
  (unsigned char)KeyCode::Unknown, // Qt::Key_Launch2	0x010000a4	On X11 this key is mapped to XF86XK_Launch0 key for legacy reasons.
  (unsigned char)KeyCode::Unknown, // Qt::Key_Launch3	0x010000a5	On X11 this key is mapped to XF86XK_Launch1 key for legacy reasons.
  (unsigned char)KeyCode::Unknown, // Qt::Key_Launch4	0x010000a6	On X11 this key is mapped to XF86XK_Launch2 key for legacy reasons.
  (unsigned char)KeyCode::Unknown, // Qt::Key_Launch5	0x010000a7	On X11 this key is mapped to XF86XK_Launch3 key for legacy reasons.
  (unsigned char)KeyCode::Unknown, // Qt::Key_Launch6	0x010000a8	On X11 this key is mapped to XF86XK_Launch4 key for legacy reasons.
  (unsigned char)KeyCode::Unknown, // Qt::Key_Launch7	0x010000a9	On X11 this key is mapped to XF86XK_Launch5 key for legacy reasons.
  (unsigned char)KeyCode::Unknown, // Qt::Key_Launch8	0x010000aa	On X11 this key is mapped to XF86XK_Launch6 key for legacy reasons.
  (unsigned char)KeyCode::Unknown, // Qt::Key_Launch9	0x010000ab	On X11 this key is mapped to XF86XK_Launch7 key for legacy reasons.
  (unsigned char)KeyCode::Unknown, // Qt::Key_LaunchA	0x010000ac	On X11 this key is mapped to XF86XK_Launch8 key for legacy reasons.
  (unsigned char)KeyCode::Unknown, // Qt::Key_LaunchB	0x010000ad	On X11 this key is mapped to XF86XK_Launch9 key for legacy reasons.
  (unsigned char)KeyCode::Unknown, // Qt::Key_LaunchC	0x010000ae	On X11 this key is mapped to XF86XK_LaunchA key for legacy reasons.
  (unsigned char)KeyCode::Unknown, // Qt::Key_LaunchD	0x010000af	On X11 this key is mapped to XF86XK_LaunchB key for legacy reasons.
  (unsigned char)KeyCode::Unknown, // Qt::Key_LaunchE	0x010000b0	On X11 this key is mapped to XF86XK_LaunchC key for legacy reasons.
  (unsigned char)KeyCode::Unknown, // Qt::Key_LaunchF	0x010000b1	On X11 this key is mapped to XF86XK_LaunchD key for legacy reasons.
  (unsigned char)KeyCode::Unknown, // Qt::Key_MonBrightnessUp	0x010000b2
  (unsigned char)KeyCode::Unknown, // Qt::Key_MonBrightnessDown	0x010000b3
  (unsigned char)KeyCode::Unknown, // Qt::Key_KeyboardLightOnOff	0x010000b4
  (unsigned char)KeyCode::Unknown, // Qt::Key_KeyboardBrightnessUp	0x010000b5
  (unsigned char)KeyCode::Unknown, // Qt::Key_KeyboardBrightnessDown	0x010000b6
  (unsigned char)KeyCode::Unknown, // Qt::Key_PowerOff	0x010000b7
  (unsigned char)KeyCode::Unknown, // Qt::Key_WakeUp	0x010000b8
  (unsigned char)KeyCode::Unknown, // Qt::Key_Eject	0x010000b9
  (unsigned char)KeyCode::Unknown, // Qt::Key_ScreenSaver	0x010000ba
  (unsigned char)KeyCode::Unknown, // Qt::Key_WWW	0x010000bb
  (unsigned char)KeyCode::Unknown, // Qt::Key_Memo	0x010000bc
  (unsigned char)KeyCode::Unknown, // Qt::Key_LightBulb	0x010000bd
  (unsigned char)KeyCode::Unknown, // Qt::Key_Shop	0x010000be
  (unsigned char)KeyCode::Unknown, // Qt::Key_History	0x010000bf
  (unsigned char)KeyCode::Unknown, // Qt::Key_AddFavorite	0x010000c0
  (unsigned char)KeyCode::Unknown, // Qt::Key_HotLinks	0x010000c1
  (unsigned char)KeyCode::Unknown, // Qt::Key_BrightnessAdjust	0x010000c2
  (unsigned char)KeyCode::Unknown, // Qt::Key_Finance	0x010000c3
  (unsigned char)KeyCode::Unknown, // Qt::Key_Community	0x010000c4
  (unsigned char)KeyCode::Unknown, // Qt::Key_AudioRewind	0x010000c5
  (unsigned char)KeyCode::Unknown, // Qt::Key_BackForward	0x010000c6
  (unsigned char)KeyCode::Unknown, // Qt::Key_ApplicationLeft	0x010000c7
  (unsigned char)KeyCode::Unknown, // Qt::Key_ApplicationRight	0x010000c8
  (unsigned char)KeyCode::Unknown, // Qt::Key_Book	0x010000c9
  (unsigned char)KeyCode::Unknown, // Qt::Key_CD	0x010000ca
  (unsigned char)KeyCode::Unknown, // Qt::Key_Calculator	0x010000cb	On X11 this key is not mapped for legacy reasons.Use Qt::Key_Launch1 instead.
  (unsigned char)KeyCode::Unknown, // Qt::Key_ToDoList	0x010000cc
  (unsigned char)KeyCode::Unknown, // Qt::Key_ClearGrab	0x010000cd
  (unsigned char)KeyCode::Unknown, // Qt::Key_Close	0x010000ce
  (unsigned char)KeyCode::Unknown, // Qt::Key_Copy	0x010000cf
  (unsigned char)KeyCode::Unknown, // Qt::Key_Cut	0x010000d0
  (unsigned char)KeyCode::Unknown, // Qt::Key_Display	0x010000d1
  (unsigned char)KeyCode::Unknown, // Qt::Key_DOS	0x010000d2
  (unsigned char)KeyCode::Unknown, // Qt::Key_Documents	0x010000d3
  (unsigned char)KeyCode::Unknown, // Qt::Key_Excel	0x010000d4
  (unsigned char)KeyCode::Unknown, // Qt::Key_Explorer	0x010000d5
  (unsigned char)KeyCode::Unknown, // Qt::Key_Game	0x010000d6
  (unsigned char)KeyCode::Unknown, // Qt::Key_Go	0x010000d7
  (unsigned char)KeyCode::Unknown, // Qt::Key_iTouch	0x010000d8
  (unsigned char)KeyCode::Unknown, // Qt::Key_LogOff	0x010000d9
  (unsigned char)KeyCode::Unknown, // Qt::Key_Market	0x010000da
  (unsigned char)KeyCode::Unknown, // Qt::Key_Meeting	0x010000db
  (unsigned char)KeyCode::Unknown, // Qt::Key_MenuKB	0x010000dc
  (unsigned char)KeyCode::Unknown, // Qt::Key_MenuPB	0x010000dd
  (unsigned char)KeyCode::Unknown, // Qt::Key_MySites	0x010000de
  (unsigned char)KeyCode::Unknown, // Qt::Key_News	0x010000df
  (unsigned char)KeyCode::Unknown, // Qt::Key_OfficeHome	0x010000e0
  (unsigned char)KeyCode::Unknown, // Qt::Key_Option	0x010000e1
  (unsigned char)KeyCode::Unknown, // Qt::Key_Paste	0x010000e2
  (unsigned char)KeyCode::Unknown, // Qt::Key_Phone	0x010000e3
  (unsigned char)KeyCode::Unknown, // Qt::Key_Calendar	0x010000e4
  (unsigned char)KeyCode::Unknown, // Qt::Key_Reply	0x010000e5
  (unsigned char)KeyCode::Unknown, // Qt::Key_Reload	0x010000e6
  (unsigned char)KeyCode::Unknown, // Qt::Key_RotateWindows	0x010000e7
  (unsigned char)KeyCode::Unknown, // Qt::Key_RotationPB	0x010000e8
  (unsigned char)KeyCode::Unknown, // Qt::Key_RotationKB	0x010000e9
  (unsigned char)KeyCode::Unknown, // Qt::Key_Save	0x010000ea
  (unsigned char)KeyCode::Unknown, // Qt::Key_Send	0x010000eb
  (unsigned char)KeyCode::Unknown, // Qt::Key_Spell	0x010000ec
  (unsigned char)KeyCode::Unknown, // Qt::Key_SplitScreen	0x010000ed
  (unsigned char)KeyCode::Unknown, // Qt::Key_Support	0x010000ee
  (unsigned char)KeyCode::Unknown, // Qt::Key_TaskPane	0x010000ef
  (unsigned char)KeyCode::Unknown, // Qt::Key_Terminal	0x010000f0
  (unsigned char)KeyCode::Unknown, // Qt::Key_Tools	0x010000f1
  (unsigned char)KeyCode::Unknown, // Qt::Key_Travel	0x010000f2
  (unsigned char)KeyCode::Unknown, // Qt::Key_Video	0x010000f3
  (unsigned char)KeyCode::Unknown, // Qt::Key_Word	0x010000f4
  (unsigned char)KeyCode::Unknown, // Qt::Key_Xfer	0x010000f5
  (unsigned char)KeyCode::Unknown, // Qt::Key_ZoomIn	0x010000f6
  (unsigned char)KeyCode::Unknown, // Qt::Key_ZoomOut	0x010000f7
  (unsigned char)KeyCode::Unknown, // Qt::Key_Away	0x010000f8
  (unsigned char)KeyCode::Unknown, // Qt::Key_Messenger	0x010000f9
  (unsigned char)KeyCode::Unknown, // Qt::Key_WebCam	0x010000fa
  (unsigned char)KeyCode::Unknown, // Qt::Key_MailForward	0x010000fb
  (unsigned char)KeyCode::Unknown, // Qt::Key_Pictures	0x010000fc
  (unsigned char)KeyCode::Unknown, // Qt::Key_Music	0x010000fd
  (unsigned char)KeyCode::Unknown, // Qt::Key_Battery	0x010000fe
  (unsigned char)KeyCode::Unknown, // Qt::Key_Bluetooth	0x010000ff
  (unsigned char)KeyCode::Unknown, // Qt::Key_WLAN	0x01000100
  (unsigned char)KeyCode::Unknown, // Qt::Key_UWB	0x01000101
  (unsigned char)KeyCode::Unknown, // Qt::Key_AudioForward	0x01000102
  (unsigned char)KeyCode::Unknown, // Qt::Key_AudioRepeat	0x01000103
  (unsigned char)KeyCode::Unknown, // Qt::Key_AudioRandomPlay	0x01000104
  (unsigned char)KeyCode::Unknown, // Qt::Key_Subtitle	0x01000105
  (unsigned char)KeyCode::Unknown, // Qt::Key_AudioCycleTrack	0x01000106
  (unsigned char)KeyCode::Unknown, // Qt::Key_Time	0x01000107
  (unsigned char)KeyCode::Unknown, // Qt::Key_Hibernate	0x01000108
  (unsigned char)KeyCode::Unknown, // Qt::Key_View	0x01000109
  (unsigned char)KeyCode::Unknown, // Qt::Key_TopMenu	0x0100010a
  (unsigned char)KeyCode::Unknown, // Qt::Key_PowerDown	0x0100010b
  (unsigned char)KeyCode::Unknown, // Qt::Key_Suspend	0x0100010c
  (unsigned char)KeyCode::Unknown, // Qt::Key_ContrastAdjust	0x0100010d
  (unsigned char)KeyCode::Unknown, // Qt::Key_LaunchG	0x0100010e	On X11 this key is mapped to XF86XK_LaunchE key for legacy reasons.
  (unsigned char)KeyCode::Unknown, // Qt::Key_LaunchH	0x0100010f	On X11 this key is mapped to XF86XK_LaunchF key for legacy reasons.
  (unsigned char)KeyCode::Unknown, // Qt::Key_TouchpadToggle	0x01000110
  (unsigned char)KeyCode::Unknown, // Qt::Key_TouchpadOn	0x01000111
  (unsigned char)KeyCode::Unknown, // Qt::Key_TouchpadOff	0x01000112
  (unsigned char)KeyCode::Unknown, // Qt::Key_MicMute	0x01000113
  (unsigned char)KeyCode::Unknown, // Qt::Key_Red	0x01000114
  (unsigned char)KeyCode::Unknown, // Qt::Key_Green	0x01000115
  (unsigned char)KeyCode::Unknown, // Qt::Key_Yellow	0x01000116
  (unsigned char)KeyCode::Unknown, // Qt::Key_Blue	0x01000117
  (unsigned char)KeyCode::Unknown, // Qt::Key_ChannelUp	0x01000118
  (unsigned char)KeyCode::Unknown, // Qt::Key_ChannelDown	0x01000119
};

KeyCode qtKeyToEPKey(Qt::Key qk)
{
  if ((qk & ~0xFF) == 0)
    return (KeyCode)qtKeyToEPKeyLower[qk - 0x20];
  else if ((qk & 0xFFFFFF00) == 0x01000000)
    return (KeyCode)qtKeyToEPKeyHighBit[qk & 0xFFF];
  else
  {
//    KeyCode::RAlt, // Qt::Key_AltGr	0x01001103	On Windows, when the KeyDown event for this key is sent, the Ctrl+Alt modifiers are also set.
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

    return KeyCode::Unknown;
  }
}

#else
EPEMPTYFILE
#endif
