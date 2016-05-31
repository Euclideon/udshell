#pragma once
#ifndef EPINPUT_H
#define EPINPUT_H

#include "ep/cpp/platform.h"
#include "ep/cpp/input.h" // TODO Note some enums and structs moved in here so they can be accessed from Libep components. HAL needs to be revisted to work out what should be available to libep

ep::InputDeviceState epInput_GetDeviceState(ep::InputDevice device, int deviceIndex);

// TODO: remove these, input is strictly event driven!
bool epInput_WasPressed(ep::InputDevice device, int control, int deviceIndex = -1);
bool epInput_WasReleased(ep::InputDevice device, int control, int deviceIndex = -1);
float epInput_State(ep::InputDevice device, int control, int deviceIndex = -1);

unsigned int epInput_LockMouseOnButtons(unsigned int buttonBits);
inline ep::KeyCode epInput_AsciiToKeyCode(unsigned char c)
{
  extern unsigned char epAsciiToEPKey[128];
  return (ep::KeyCode)epAsciiToEPKey[c];
}


// internal
void epInput_Init();
void epInput_Update();

#endif // EPINPUT_H
