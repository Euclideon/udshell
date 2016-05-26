#pragma once
#ifndef EPINPUT_H
#define EPINPUT_H

#include "ep/cpp/platform.h"
#include "ep/c/input.h" // TODO Note some enums and structs moved in here so they can be accessed from Libep components. HAL needs to be revisted to work out what should be available to libep

epInputDeviceState epInput_GetDeviceState(epInputDevice device, int deviceIndex);

// TODO: remove these, input is strictly event driven!
bool epInput_WasPressed(epInputDevice device, int control, int deviceIndex = -1);
bool epInput_WasReleased(epInputDevice device, int control, int deviceIndex = -1);
float epInput_State(epInputDevice device, int control, int deviceIndex = -1);

unsigned int epInput_LockMouseOnButtons(unsigned int buttonBits);
inline epKeyCode epInput_AsciiToKeyCode(unsigned char c)
{
  extern unsigned char epAsciiToEPKey[128];
  return (epKeyCode)epAsciiToEPKey[c];
}


// internal
void epInput_Init();
void epInput_Update();

#endif // EPINPUT_H
