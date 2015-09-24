#include "ep/epplatform.h"

#include "hal.h"

static bool bInitialised = false;
static bool bRenderInitialised = false;

udResult epHAL_Init()
{
  if (bInitialised)
    return udR_Success;

  void epInput_Init();
  epInput_Init();

  void udImage_InitInternal();
  udImage_InitInternal();

  bInitialised = true;
  return udR_Success;
}

udResult epHAL_InitRender()
{
  if (bRenderInitialised)
    return udR_Success;

  void epGPU_Init();
  epGPU_Init();

  void epDebugFont_Init();
  epDebugFont_Init();

  bRenderInitialised = true;
  return udR_Success;
}

udResult epHAL_DeinitRender()
{
  if (bRenderInitialised)
  {
    void epDebugFont_Deinit();
    epDebugFont_Deinit();

    void epGPU_Deinit();
    epGPU_Deinit();
    bRenderInitialised = false;
  }
  return udR_Success;
}

udResult epHAL_Deinit()
{
  if (bInitialised)
  {
    void udImage_DeinitInternal();
    udImage_DeinitInternal();
    void epInput_Deinit();
    epInput_Deinit();
    bInitialised = false;
  }
  return udR_Success;
}
