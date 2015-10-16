#include "hal/driver.h"

#if EPINPUT_DRIVER == EPDRIVER_GLUT

#include "udGLUT_Internal.h"
#include "udPlatformUtil.h"

#include "hal/input_internal.h"


static unsigned char fkeysToEPKey[] =
{
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
  epKC_F12
};

static unsigned char specialToEPKey[] =
{
  epKC_Left,
  epKC_Up,
  epKC_Right,
  epKC_Down,
  epKC_PageUp,
  epKC_PageDown,
  epKC_Home,
  epKC_End,
  epKC_Insert
};


InputState gInputAccum;

// --------------------------------------------------------
// Author: Manu Evans, March 2015
static void KeyPressedFunc(unsigned char key, int, int)
{
  if(epAsciiToEPKey[key] != epKC_Unknown)
    gInputAccum.keys[0][epAsciiToEPKey[key]] = 1;
}

// --------------------------------------------------------
// Author: Manu Evans, March 2015
static void KeyReleasedFunc(unsigned char key, int, int)
{
  if(epAsciiToEPKey[key] != epKC_Unknown)
    gInputAccum.keys[0][epAsciiToEPKey[key]] = 0;
}

// --------------------------------------------------------
// Author: Manu Evans, March 2015
static void SpecialKeyPressedFunc(int key, int, int)
{
  if(key >= GLUT_KEY_F1 && key <= GLUT_KEY_F12)
    gInputAccum.keys[0][fkeysToEPKey[key - GLUT_KEY_F1]] = 1;
  else if(key >= GLUT_KEY_LEFT && key <= GLUT_KEY_INSERT)
    gInputAccum.keys[0][specialToEPKey[key - GLUT_KEY_LEFT]] = 1;
}

// --------------------------------------------------------
// Author: Manu Evans, March 2015
static void SpecialKeyReleasedFunc(int key, int, int)
{
  if(key >= GLUT_KEY_F1 && key <= GLUT_KEY_F12)
    gInputAccum.keys[0][fkeysToEPKey[key - GLUT_KEY_F1]] = 0;
  else if(key >= GLUT_KEY_LEFT && key <= GLUT_KEY_INSERT)
    gInputAccum.keys[0][specialToEPKey[key - GLUT_KEY_LEFT]] = 0;
}

// --------------------------------------------------------
// Author: Manu Evans, March 2015
static void MouseFunc(int button, int state, int x, int y)
{
  if (button >= 0 && button < 4)
    gInputAccum.mouse[0][epMC_LeftButton + button] = state == GLUT_DOWN ? 1.f : 0.f;
  gInputAccum.mouse[0][epMC_XAbsolute] = (float)x;
  gInputAccum.mouse[0][epMC_YAbsolute] = (float)y;
}

// --------------------------------------------------------
// Author: Manu Evans, March 2015
static void PassiveMotionFunc(int x, int y)
{
  gInputAccum.mouse[0][epMC_XAbsolute] = (float)x;
  gInputAccum.mouse[0][epMC_YAbsolute] = (float)y;
}

// --------------------------------------------------------
// Author: Manu Evans, March 2015
void epInput_InitInternal()
{
  memset(&gInputAccum, 0, sizeof(gInputAccum));

  glutKeyboardFunc(KeyPressedFunc);
  glutKeyboardUpFunc(KeyReleasedFunc);
  glutSpecialFunc(SpecialKeyPressedFunc);
  glutSpecialUpFunc(SpecialKeyReleasedFunc);
  glutMouseFunc(MouseFunc);
  glutMotionFunc(PassiveMotionFunc);
  glutPassiveMotionFunc(PassiveMotionFunc);
}

// --------------------------------------------------------
// Author: Manu Evans, March 2015
void epInput_UpdateInternal()
{
  InputState &input = gInputState[gCurrentInputState];
  InputState &prev = gInputState[1 - gCurrentInputState];

  input = gInputAccum;

  // update deltas
  input.mouse[0][epMC_XDelta] = input.mouse[0][epMC_XAbsolute] - prev.mouse[0][epMC_XAbsolute];
  input.mouse[0][epMC_YDelta] = input.mouse[0][epMC_YAbsolute] - prev.mouse[0][epMC_YAbsolute];

  // poll gamepads
  //...

  // Fix up numpad
  input.keys[0][epKC_NumpadMinus]   = input.keys[0][epKC_Hyphen];
  input.keys[0][epKC_NumpadDivide]  = input.keys[0][epKC_ForwardSlash];
  input.keys[0][epKC_NumpadDecimal] = input.keys[0][epKC_Period];
  input.keys[0][epKC_NumpadEnter]   = input.keys[0][epKC_Enter];

  input.keys[0][epKC_Numpad0]   = input.keys[0][epKC_0];
  input.keys[0][epKC_Numpad1]   = input.keys[0][epKC_1];
  input.keys[0][epKC_Numpad2]   = input.keys[0][epKC_2];
  input.keys[0][epKC_Numpad3]   = input.keys[0][epKC_3];
  input.keys[0][epKC_Numpad4]   = input.keys[0][epKC_4];

  input.keys[0][epKC_Numpad5]   = input.keys[0][epKC_5];
  input.keys[0][epKC_Numpad6]   = input.keys[0][epKC_6];
  input.keys[0][epKC_Numpad7]   = input.keys[0][epKC_7];
  input.keys[0][epKC_Numpad8]   = input.keys[0][epKC_8];
  input.keys[0][epKC_Numpad9]   = input.keys[0][epKC_9];


}

#else
EPEMPTYFILE
#endif
