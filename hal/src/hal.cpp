#include "ep/cpp/platform.h"

#include "hal/hal.h"

using namespace ep;

static bool bInitialised = false;
static bool bRenderInitialised = false;

Result epHAL_Init()
{
  if (bInitialised)
    return Result::Success;

  void epInput_Init();
  epInput_Init();

  void epImage_InitInternal();
  epImage_InitInternal();

  bInitialised = true;
  return Result::Success;
}

Result epHAL_InitRender()
{
  if (bRenderInitialised)
    return Result::Success;

  void epGPU_Init();
  epGPU_Init();

  void epDebugFont_Init();
  epDebugFont_Init();

  bRenderInitialised = true;
  return Result::Success;
}

Result epHAL_DeinitRender()
{
  if (bRenderInitialised)
  {
    void epDebugFont_Deinit();
    epDebugFont_Deinit();

    void epGPU_Deinit();
    epGPU_Deinit();
    bRenderInitialised = false;
  }
  return Result::Success;
}

Result epHAL_Deinit()
{
  if (bInitialised)
  {
    void epImage_DeinitInternal();
    epImage_DeinitInternal();
    void epInput_Deinit();
    epInput_Deinit();
    bInitialised = false;
  }
  return Result::Success;
}
