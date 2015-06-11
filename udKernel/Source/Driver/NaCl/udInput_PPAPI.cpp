#include "udDriver.h"

#if UDINPUT_DRIVER == UDDRIVER_PPAPI

#include "../../udInput_Internal.h"


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
  //...

  // poll mouse
  //...

  // poll gamepads
  //...
}

#endif
