#include "input_internal.h"

using namespace ep;

const int gMaxDevices[InputDevice::Max] =
{
  MAX_KEYBOARDS, // InputDevice::Keyboard
  MAX_MOUSES, // InputDevice::Mouse
  MAX_GAMEPADS, // InputDevice::Gamepad
//  1, // InputDevice::TouchScreen
//  1, // InputDevice::Accelerometer
//  1  // InputDevice::Compas
};

//static InputState liveState;
InputState gInputState[2];
int gCurrentInputState = 0;

static unsigned int mouseLock = 0;
#if 0
static unsigned int mouseLocked = 0;
static int ignoreMouseEvents = 0;
#endif // 0

// ********************************************************
// Author: Manu Evans, March 2015
InputDeviceState epInput_GetDeviceState(InputDevice, int)
{
  return InputDeviceState::Ready;
}

// ********************************************************
// Author: Manu Evans, March 2015
unsigned int epInput_LockMouseOnButtons(unsigned int buttonBits)
{
  unsigned int old = mouseLock;
  mouseLock = buttonBits;
  return old;
}

// ********************************************************
// Author: Manu Evans, March 2015
bool epInput_WasPressed(InputDevice device, int control, int deviceIndex)
{
  if (deviceIndex == -1)
  {
    for (int i=0; i<gMaxDevices[device]; ++i)
      if(epInput_WasPressed(device, control, i))
        return true;
    return false;
  }

  InputState prev = gInputState[1-gCurrentInputState];
  InputState state = gInputState[gCurrentInputState];
  bool result = false;
  switch (device)
  {
  case InputDevice::Keyboard:
      result = state.keys[deviceIndex][control] && !prev.keys[deviceIndex][control];
      break;
    case InputDevice::Mouse:
      result = state.mouse[deviceIndex][control] && !prev.mouse[deviceIndex][control];;
      break;
    case InputDevice::Gamepad:
      result = state.gamepad[deviceIndex][control] && !prev.gamepad[deviceIndex][control];
      break;
    default:
      EPUNREACHABLE;
  }
  return result;
}

// ********************************************************
// Author: Manu Evans, March 2015
bool epInput_WasReleased(InputDevice device, int control, int deviceIndex)
{
  if (deviceIndex == -1)
  {
    for (int i=0; i<gMaxDevices[device]; ++i)
      if(epInput_WasReleased(device, control, i))
        return true;
    return false;
  }

  InputState prev = gInputState[1-gCurrentInputState];
  InputState state = gInputState[gCurrentInputState];
  bool result = false;
  switch (device)
  {
    case InputDevice::Keyboard:
      result = !state.keys[deviceIndex][control] && prev.keys[deviceIndex][control];
      break;
    case InputDevice::Mouse:
      result = !state.mouse[deviceIndex][control] && prev.mouse[deviceIndex][control];
      break;
    case InputDevice::Gamepad:
      result = !state.gamepad[deviceIndex][control] && prev.gamepad[deviceIndex][control];
      break;
    default:
      EPUNREACHABLE;
  }
  return result;
}

// ********************************************************
// Author: Manu Evans, March 2015
float epInput_State(InputDevice device, int control, int deviceIndex)
{
  if (deviceIndex == -1)
  {
    float state = 0.f;
    for (int i=0; i<gMaxDevices[device]; ++i)
      state += epInput_State(device, control, i);
    return state;
  }

  InputState state = gInputState[gCurrentInputState];
  float result = 0.0f;
  switch (device)
  {
    case InputDevice::Keyboard:
      result = state.keys[deviceIndex][control] ? 1.f : 0.f;
      break;
    case InputDevice::Mouse:
      result = state.mouse[deviceIndex][control];
      break;
    case InputDevice::Gamepad:
      result = state.gamepad[deviceIndex][control];
      break;
      //    case InputDevice::TouchScreen:
//    case InputDevice::Accelerometer:
//    case InputDevice::Compas:
      // etc...
    default:
      EPUNREACHABLE;
  }
  return result;
}

// ********************************************************
// Author: Manu Evans, March 2015
void epInput_Init()
{
  epInput_InitInternal();
}

// ********************************************************
// Author: Manu Evans, March 2015
void epInput_Deinit()
{
  // TODO: deinit internal?
}

// ********************************************************
// Author: Manu Evans, March 2015
void epInput_Update()
{
  // switch input frame
  gCurrentInputState = 1 - gCurrentInputState;

  epInput_UpdateInternal();
}


unsigned char epAsciiToEPKey[128] =
{
  (unsigned char)KeyCode::Unknown, // 0	000	00	00000000	NUL	&#000;	 	Null char
  (unsigned char)KeyCode::Unknown, // 1	001	01	00000001	SOH	&#001;	 	Start of Heading
  (unsigned char)KeyCode::Unknown, // 2	002	02	00000010	STX	&#002;	 	Start of Text
  (unsigned char)KeyCode::Unknown, // 3	003	03	00000011	ETX	&#003;	 	End of Text
  (unsigned char)KeyCode::Unknown, // 4	004	04	00000100	EOT	&#004;	 	End of Transmission
  (unsigned char)KeyCode::Unknown, // 5	005	05	00000101	ENQ	&#005;	 	Enquiry
  (unsigned char)KeyCode::Unknown, // 6	006	06	00000110	ACK	&#006;	 	Acknowledgment
  (unsigned char)KeyCode::Unknown, // 7	007	07	00000111	BEL	&#007;	 	Bell
  (unsigned char)KeyCode::Backspace, // 8	010	08	00001000	BS	&#008;	 	Back Space
  (unsigned char)KeyCode::Tab, // 9	011	09	00001001	HT	&#009;	 	Horizontal Tab
  (unsigned char)KeyCode::Unknown, // 10	012	0A	00001010	LF	&#010;	 	Line Feed
  (unsigned char)KeyCode::Unknown, // 11	013	0B	00001011	VT	&#011;	 	Vertical Tab
  (unsigned char)KeyCode::Unknown, // 12	014	0C	00001100	FF	&#012;	 	Form Feed
  (unsigned char)KeyCode::Enter, // 13	015	0D	00001101	CR	&#013;	 	Carriage Return
  (unsigned char)KeyCode::Unknown, // 14	016	0E	00001110	SO	&#014;	 	Shift Out / X-On
  (unsigned char)KeyCode::Unknown, // 15	017	0F	00001111	SI	&#015;	 	Shift In / X-Off
  (unsigned char)KeyCode::Unknown, // 16	020	10	00010000	DLE	&#016;	 	Data Line Escape
  (unsigned char)KeyCode::Unknown, // 17	021	11	00010001	DC1	&#017;	 	Device Control 1 (oft. XON)
  (unsigned char)KeyCode::Unknown, // 18	022	12	00010010	DC2	&#018;	 	Device Control 2
  (unsigned char)KeyCode::Unknown, // 19	023	13	00010011	DC3	&#019;	 	Device Control 3 (oft. XOFF)
  (unsigned char)KeyCode::Unknown, // 20	024	14	00010100	DC4	&#020;	 	Device Control 4
  (unsigned char)KeyCode::Unknown, // 21	025	15	00010101	NAK	&#021;	 	Negative Acknowledgement
  (unsigned char)KeyCode::Unknown, // 22	026	16	00010110	SYN	&#022;	 	Synchronous Idle
  (unsigned char)KeyCode::Unknown, // 23	027	17	00010111	ETB	&#023;	 	End of Transmit Block
  (unsigned char)KeyCode::Unknown, // 24	030	18	00011000	CAN	&#024;	 	Cancel
  (unsigned char)KeyCode::Unknown, // 25	031	19	00011001	EM	&#025;	 	End of Medium
  (unsigned char)KeyCode::Unknown, // 26	032	1A	00011010	SUB	&#026;	 	Substitute
  (unsigned char)KeyCode::Escape, // 27	033	1B	00011011	ESC	&#027;	 	Escape
  (unsigned char)KeyCode::Unknown, // 28	034	1C	00011100	FS	&#028;	 	File Separator
  (unsigned char)KeyCode::Unknown, // 29	035	1D	00011101	GS	&#029;	 	Group Separator
  (unsigned char)KeyCode::Unknown, // 30	036	1E	00011110	RS	&#030;	 	Record Separator
  (unsigned char)KeyCode::Unknown, // 31	037	1F	00011111	US	&#031;	 	Unit Separator
  (unsigned char)KeyCode::Space, // 32	040	20	00100000	 	&#32;	 	Space
  (unsigned char)KeyCode::Unknown, // 33	041	21	00100001	!	&#33;	 	Exclamation mark
  (unsigned char)KeyCode::Unknown, // 34	042	22	00100010	"	&#34;	&quot;	Double quotes (or speech marks)
  (unsigned char)KeyCode::Unknown, // 35	043	23	00100011	#	&#35;	 	Number
  (unsigned char)KeyCode::Unknown, // 36	044	24	00100100	$	&#36;	 	Dollar
  (unsigned char)KeyCode::Unknown, // 37	045	25	00100101	%	&#37;	 	Procenttecken
  (unsigned char)KeyCode::Unknown, // 38	046	26	00100110	&	&#38;	&amp;	Ampersand
  (unsigned char)KeyCode::Apostrophe, // 39	047	27	00100111	'	&#39;	 	Single quote
  (unsigned char)KeyCode::Unknown, // 40	050	28	00101000	(	&#40;	 	Open parenthesis (or open bracket)
  (unsigned char)KeyCode::Unknown, // 41	051	29	00101001	)	&#41;	 	Close parenthesis (or close bracket)
  (unsigned char)KeyCode::NumpadMultiply, // 42	052	2A	00101010	*	&#42;	 	Asterisk
  (unsigned char)KeyCode::NumpadPlus, // 43	053	2B	00101011	+	&#43;	 	Plus
  (unsigned char)KeyCode::Comma, // 44	054	2C	00101100	,	&#44;	 	Comma
  (unsigned char)KeyCode::Hyphen, // 45	055	2D	00101101	-	&#45;	 	Hyphen
  (unsigned char)KeyCode::Period, // 46	056	2E	00101110	.	&#46;	 	Period, dot or full stop
  (unsigned char)KeyCode::ForwardSlash, // 47	057	2F	00101111	/	&#47;	 	Slash or divide
  (unsigned char)KeyCode::_0, // 48	060	30	00110000	0	&#48;	 	Zero
  (unsigned char)KeyCode::_1, // 49	061	31	00110001	1	&#49;	 	One
  (unsigned char)KeyCode::_2, // 50	062	32	00110010	2	&#50;	 	Two
  (unsigned char)KeyCode::_3, // 51	063	33	00110011	3	&#51;	 	Three
  (unsigned char)KeyCode::_4, // 52	064	34	00110100	4	&#52;	 	Four
  (unsigned char)KeyCode::_5, // 53	065	35	00110101	5	&#53;	 	Five
  (unsigned char)KeyCode::_6, // 54	066	36	00110110	6	&#54;	 	Six
  (unsigned char)KeyCode::_7, // 55	067	37	00110111	7	&#55;	 	Seven
  (unsigned char)KeyCode::_8, // 56	070	38	00111000	8	&#56;	 	Eight
  (unsigned char)KeyCode::_9, // 57	071	39	00111001	9	&#57;	 	Nine
  (unsigned char)KeyCode::Unknown, // 58	072	3A	00111010	:	&#58;	 	Colon
  (unsigned char)KeyCode::Semicolon, // 59	073	3B	00111011	;	&#59;	 	Semicolon
  (unsigned char)KeyCode::Unknown, // 60	074	3C	00111100	<	&#60;	&lt;	Less than (or open angled bracket)
  (unsigned char)KeyCode::Equals, // 61	075	3D	00111101	=	&#61;	 	Equals
  (unsigned char)KeyCode::Unknown, // 62	076	3E	00111110	>	&#62;	&gt;	Greater than (or close angled bracket)
  (unsigned char)KeyCode::Unknown, // 63	077	3F	00111111	?	&#63;	 	Question mark
  (unsigned char)KeyCode::Unknown, // 64	100	40	01000000	@	&#64;	 	At symbol
  (unsigned char)KeyCode::A, // 65	101	41	01000001	A	&#65;	 	Uppercase A
  (unsigned char)KeyCode::B, // 66	102	42	01000010	B	&#66;	 	Uppercase B
  (unsigned char)KeyCode::C, // 67	103	43	01000011	C	&#67;	 	Uppercase C
  (unsigned char)KeyCode::D, // 68	104	44	01000100	D	&#68;	 	Uppercase D
  (unsigned char)KeyCode::E, // 69	105	45	01000101	E	&#69;	 	Uppercase E
  (unsigned char)KeyCode::F, // 70	106	46	01000110	F	&#70;	 	Uppercase F
  (unsigned char)KeyCode::G, // 71	107	47	01000111	G	&#71;	 	Uppercase G
  (unsigned char)KeyCode::H, // 72	110	48	01001000	H	&#72;	 	Uppercase H
  (unsigned char)KeyCode::I, // 73	111	49	01001001	I	&#73;	 	Uppercase I
  (unsigned char)KeyCode::J, // 74	112	4A	01001010	J	&#74;	 	Uppercase J
  (unsigned char)KeyCode::K, // 75	113	4B	01001011	K	&#75;	 	Uppercase K
  (unsigned char)KeyCode::L, // 76	114	4C	01001100	L	&#76;	 	Uppercase L
  (unsigned char)KeyCode::M, // 77	115	4D	01001101	M	&#77;	 	Uppercase M
  (unsigned char)KeyCode::N, // 78	116	4E	01001110	N	&#78;	 	Uppercase N
  (unsigned char)KeyCode::O, // 79	117	4F	01001111	O	&#79;	 	Uppercase O
  (unsigned char)KeyCode::P, // 80	120	50	01010000	P	&#80;	 	Uppercase P
  (unsigned char)KeyCode::Q, // 81	121	51	01010001	Q	&#81;	 	Uppercase Q
  (unsigned char)KeyCode::R, // 82	122	52	01010010	R	&#82;	 	Uppercase R
  (unsigned char)KeyCode::S, // 83	123	53	01010011	S	&#83;	 	Uppercase S
  (unsigned char)KeyCode::T, // 84	124	54	01010100	T	&#84;	 	Uppercase T
  (unsigned char)KeyCode::U, // 85	125	55	01010101	U	&#85;	 	Uppercase U
  (unsigned char)KeyCode::V, // 86	126	56	01010110	V	&#86;	 	Uppercase V
  (unsigned char)KeyCode::W, // 87	127	57	01010111	W	&#87;	 	Uppercase W
  (unsigned char)KeyCode::X, // 88	130	58	01011000	X	&#88;	 	Uppercase X
  (unsigned char)KeyCode::Y, // 89	131	59	01011001	Y	&#89;	 	Uppercase Y
  (unsigned char)KeyCode::Z, // 90	132	5A	01011010	Z	&#90;	 	Uppercase Z
  (unsigned char)KeyCode::LeftBracket, // 91	133	5B	01011011	[	&#91;	 	Opening bracket
  (unsigned char)KeyCode::BackSlash, // 92	134	5C	01011100	\	&#92;	 	Backslash
  (unsigned char)KeyCode::RightBracket, // 93	135	5D	01011101	]	&#93;	 	Closing bracket
  (unsigned char)KeyCode::Unknown, // 94	136	5E	01011110	^	&#94;	 	Caret - circumflex
  (unsigned char)KeyCode::Unknown, // 95	137	5F	01011111	_	&#95;	 	Underscore
  (unsigned char)KeyCode::Grave, // 96	140	60	01100000	`	&#96;	 	Grave accent
  (unsigned char)KeyCode::A, // 97	141	61	01100001	a	&#97;	 	Lowercase a
  (unsigned char)KeyCode::B, // 98	142	62	01100010	b	&#98;	 	Lowercase b
  (unsigned char)KeyCode::C, // 99	143	63	01100011	c	&#99;	 	Lowercase c
  (unsigned char)KeyCode::D, // 100	144	64	01100100	d	&#100;	 	Lowercase d
  (unsigned char)KeyCode::E, // 101	145	65	01100101	e	&#101;	 	Lowercase e
  (unsigned char)KeyCode::F, // 102	146	66	01100110	f	&#102;	 	Lowercase f
  (unsigned char)KeyCode::G, // 103	147	67	01100111	g	&#103;	 	Lowercase g
  (unsigned char)KeyCode::H, // 104	150	68	01101000	h	&#104;	 	Lowercase h
  (unsigned char)KeyCode::I, // 105	151	69	01101001	i	&#105;	 	Lowercase i
  (unsigned char)KeyCode::J, // 106	152	6A	01101010	j	&#106;	 	Lowercase j
  (unsigned char)KeyCode::K, // 107	153	6B	01101011	k	&#107;	 	Lowercase k
  (unsigned char)KeyCode::L, // 108	154	6C	01101100	l	&#108;	 	Lowercase l
  (unsigned char)KeyCode::M, // 109	155	6D	01101101	m	&#109;	 	Lowercase m
  (unsigned char)KeyCode::N, // 110	156	6E	01101110	n	&#110;	 	Lowercase n
  (unsigned char)KeyCode::O, // 111	157	6F	01101111	o	&#111;	 	Lowercase o
  (unsigned char)KeyCode::P, // 112	160	70	01110000	p	&#112;	 	Lowercase p
  (unsigned char)KeyCode::Q, // 113	161	71	01110001	q	&#113;	 	Lowercase q
  (unsigned char)KeyCode::R, // 114	162	72	01110010	r	&#114;	 	Lowercase r
  (unsigned char)KeyCode::S, // 115	163	73	01110011	s	&#115;	 	Lowercase s
  (unsigned char)KeyCode::T, // 116	164	74	01110100	t	&#116;	 	Lowercase t
  (unsigned char)KeyCode::U, // 117	165	75	01110101	u	&#117;	 	Lowercase u
  (unsigned char)KeyCode::V, // 118	166	76	01110110	v	&#118;	 	Lowercase v
  (unsigned char)KeyCode::W, // 119	167	77	01110111	w	&#119;	 	Lowercase w
  (unsigned char)KeyCode::X, // 120	170	78	01111000	x	&#120;	 	Lowercase x
  (unsigned char)KeyCode::Y, // 121	171	79	01111001	y	&#121;	 	Lowercase y
  (unsigned char)KeyCode::Z, // 122	172	7A	01111010	z	&#122;	 	Lowercase z
  (unsigned char)KeyCode::Unknown, // 123	173	7B	01111011	{	&#123;	 	Opening brace
  (unsigned char)KeyCode::Unknown, // 124	174	7C	01111100	|	&#124;	 	Vertical bar
  (unsigned char)KeyCode::Unknown, // 125	175	7D	01111101	}	&#125;	 	Closing brace
  (unsigned char)KeyCode::Unknown, // 126	176	7E	01111110	~	&#126;	 	Equivalency sign - tilde
  (unsigned char)KeyCode::Delete // 127	177	7F	01111111		&#127;	 	Delete
};
