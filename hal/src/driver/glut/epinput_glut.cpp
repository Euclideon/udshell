#include "driver.h"

#if EPINPUT_DRIVER == EPDRIVER_GLUT

#include "udGLUT_Internal.h"
#include "udPlatformUtil.h"

#include "input_internal.h"

using namespace ep;


static unsigned char fkeysToEPKey[] =
{
  KeyCode::F1,
  KeyCode::F2,
  KeyCode::F3,
  KeyCode::F4,
  KeyCode::F5,
  KeyCode::F6,
  KeyCode::F7,
  KeyCode::F8,
  KeyCode::F9,
  KeyCode::F10,
  KeyCode::F11,
  KeyCode::F12
};

static unsigned char specialToEPKey[] =
{
  KeyCode::Left,
  KeyCode::Up,
  KeyCode::Right,
  KeyCode::Down,
  KeyCode::PageUp,
  KeyCode::PageDown,
  KeyCode::Home,
  KeyCode::End,
  KeyCode::Insert
};


InputState gInputAccum;

// --------------------------------------------------------
// Author: Manu Evans, March 2015
static void KeyPressedFunc(unsigned char key, int, int)
{
  if((KeyCode)epAsciiToEPKey[key] != KeyCode::Unknown)
    gInputAccum.keys[0][epAsciiToEPKey[key]] = 1;
}

// --------------------------------------------------------
// Author: Manu Evans, March 2015
static void KeyReleasedFunc(unsigned char key, int, int)
{
  if((KeyCode)epAsciiToEPKey[key] != KeyCode::Unknown)
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
    gInputAccum.mouse[0][MouseControls::LeftButton + button] = state == GLUT_DOWN ? 1.f : 0.f;
  gInputAccum.mouse[0][MouseControls::XAbsolute] = (float)x;
  gInputAccum.mouse[0][MouseControls::YAbsolute] = (float)y;
}

// --------------------------------------------------------
// Author: Manu Evans, March 2015
static void PassiveMotionFunc(int x, int y)
{
  gInputAccum.mouse[0][MouseControls::XAbsolute] = (float)x;
  gInputAccum.mouse[0][MouseControls::YAbsolute] = (float)y;
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
  input.mouse[0][MouseControls::XDelta] = input.mouse[0][MouseControls::XAbsolute] - prev.mouse[0][MouseControls::XAbsolute];
  input.mouse[0][MouseControls::YDelta] = input.mouse[0][MouseControls::YAbsolute] - prev.mouse[0][MouseControls::YAbsolute];

  // poll gamepads
  //...

  // Fix up numpad
  input.keys[0][KeyCode::NumpadMinus]   = input.keys[0][KeyCode::Hyphen];
  input.keys[0][KeyCode::NumpadDivide]  = input.keys[0][KeyCode::ForwardSlash];
  input.keys[0][KeyCode::NumpadDecimal] = input.keys[0][KeyCode::Period];
  input.keys[0][KeyCode::NumpadEnter]   = input.keys[0][KeyCode::Enter];

  input.keys[0][KeyCode::Numpad0]   = input.keys[0][KeyCode::N0];
  input.keys[0][KeyCode::Numpad1]   = input.keys[0][KeyCode::N1];
  input.keys[0][KeyCode::Numpad2]   = input.keys[0][KeyCode::N2];
  input.keys[0][KeyCode::Numpad3]   = input.keys[0][KeyCode::N3];
  input.keys[0][KeyCode::Numpad4]   = input.keys[0][KeyCode::N4];

  input.keys[0][KeyCode::Numpad5]   = input.keys[0][KeyCode::N5];
  input.keys[0][KeyCode::Numpad6]   = input.keys[0][KeyCode::N6];
  input.keys[0][KeyCode::Numpad7]   = input.keys[0][KeyCode::N7];
  input.keys[0][KeyCode::Numpad8]   = input.keys[0][KeyCode::N8];
  input.keys[0][KeyCode::Numpad9]   = input.keys[0][KeyCode::N9];


}

#else
EPEMPTYFILE
#endif
