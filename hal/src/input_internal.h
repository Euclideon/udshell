#include "hal/input.h"

#define MAX_KEYBOARDS 1
#define MAX_MOUSES 1
#define MAX_GAMEPADS 4

struct InputState
{
  uint8_t keys[MAX_KEYBOARDS][(size_t)ep::KeyCode::Max];
  float mouse[MAX_MOUSES][(size_t)ep::MouseControls::Max];
  float gamepad[MAX_GAMEPADS][(size_t)ep::GamepadControl::Max];
};

extern InputState gInputState[2];
extern int gCurrentInputState;

extern unsigned char epAsciiToEPKey[128];

void epInput_Init();
void epInput_Update();

void epInput_InitInternal();
void epInput_UpdateInternal();
