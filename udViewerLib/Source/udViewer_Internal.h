#include "udViewer.h"
#include "udRender.h"

struct udViewerInstance
{
  udViewerInitParams initParams;

  udViewerInstanceData data;

  struct udTexture *pColorTexture;
  struct udTexture *pDepthTexture;

  udSemaphore *pRenderSemaphore;
  udSemaphore *pPresentSemaphore;

  udRenderOptions options;
  udRenderModel renderModels[16];
  size_t numRenderModels;
};

void udViewer_MainLoop();
void udViewer_BeginFrame();
void udViewer_EndFrame();
void udViewer_Update();
void udViewer_Draw();

void udViewer_ResizeFrame(int width, int height);

udViewerInstance* udViewer_GetCurrentInstance();
void udViewer_SetCurrentInstance(udViewerInstance* pInstance);

udViewerInstance* udViewerDriver_CreateInstance();
void udViewerDriver_Init(udViewerInstance *pInstance);
void udViewerDriver_Deinit(udViewerInstance *pInstance);
void udViewerDriver_RunMainLoop(udViewerInstance *pInstance);
void udViewerDriver_Quit(udViewerInstance *pInstance);

void udGPU_Init();
