#include "udPlatform.h"

#include "hal.h"

static bool bInitialised = false;
static bool bRenderInitialised = false;

udResult udHAL_Init()
{
  if (bInitialised)
    return udR_Success;

  void udInput_Init();
  udInput_Init();

  bInitialised = true;
  return udR_Success;
}

udResult udHAL_InitRender()
{
  if (bRenderInitialised)
    return udR_Success;

  void udGPU_Init();
  udGPU_Init();

  void udDebugFont_Init();
  udDebugFont_Init();

  bRenderInitialised = true;
  return udR_Success;
}

udResult udHAL_DeinitRender()
{
  if (bRenderInitialised)
  {
    void udDebugFont_Deinit();
    udDebugFont_Deinit();

    void udGPU_Deinit();
    udGPU_Deinit();
    bRenderInitialised = false;
  }
  return udR_Success;
}

udResult udHAL_Deinit()
{
  if (bInitialised)
  {
    void udInput_Deinit();
    udInput_Deinit();
    bInitialised = false;
  }
  return udR_Success;
}
