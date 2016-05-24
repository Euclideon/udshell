#include "driver.h"

#if EPINPUT_DRIVER == EPDRIVER_PPAPI

#include "input_internal.h"


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

  // Temp hack to stop warnings
  epUnused(input);
  epUnused(prev);

  // poll keyboard
  //...

  // poll mouse
  //...

  // poll gamepads
  //...
}

#else
EPEMPTYFILE
#endif
