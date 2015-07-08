#pragma once
#ifndef UDVIEW_H
#define UDVIEW_H

#include "udComponent.h"
#include "udInput.h"
#include "udRender.h"
#include "udMath.h"

PROTOTYPE_COMPONENT(udScene);
PROTOTYPE_COMPONENT(udCamera);
PROTOTYPE_COMPONENT(udView);


class udRenderableView
{
public:
  class udRenderScene *pScene; // ref counted pointer to the most recent renderable scene...

  udDouble4x4 view;
  udDouble4x4 projection;

  int displayWidth = 0, displayHeight = 0;
  int renderWidth = 0, renderHeight = 0;

protected:
  int refCount;
};

class udView : public udComponent
{
public:
  UD_COMPONENT(udView);

  virtual udResult InputEvent(const udInputEvent &ev);
  virtual udResult Resize(int width, int height);

  udResult Render(); // TODO: REMOVE ME
  udRenderView *GetUDRenderView() { return pRenderView; } // TODO: REMOVE ME

  udRenderableView *GetRenderableView(); // TODO: return an immutable object that the render thread can render

  void SetScene(udSceneRef pScene) { this->pScene = pScene; }
  void SetCamera(udCameraRef pCamera) { this->pCamera = pCamera; }
  udSceneRef GetScene() const { return pScene; }
  udCameraRef GetCamera() const { return pCamera; }

  void GetDimensions(int *pWidth, int *pHeight) const;
  void GetRenderDimensions(int *pWidth, int *pHeight) const;
  float GetAspectRatio() const { return (float)displayWidth / (float)displayHeight; }

  void* GetColorBuffer() const { return pColorBuffer; }
  void* GetDepthBuffer() const { return pDepthBuffer; }

  // TODO: remove these!
  void RegisterResizeCallback(void (*pCallback)(udViewRef pView, int w, int h)) { pResizeCallback = pCallback; }
  void RegisterPreRenderCallback(void(*pCallback)(udViewRef pView, udSceneRef pScene)) { pPreRenderCallback = pCallback; }
  void RegisterPostRenderCallback(void(*pCallback)(udViewRef pView, udSceneRef pScene)) { pPostRenderCallback = pCallback; }

protected:
  udSceneRef pScene;
  udCameraRef pCamera;

  udRenderableView *pRenderableView = nullptr;

  // --- possibly derived stuff? ---
  struct udRenderView *pRenderView = nullptr;

  void *pColorBuffer = nullptr;
  void *pDepthBuffer = nullptr;
  int displayWidth = 0, displayHeight = 0;
  int renderWidth = 0, renderHeight = 0;

  struct udTexture *pColorTexture = nullptr;
  struct udTexture *pDepthTexture = nullptr;

  udSemaphore *pRenderSemaphore = nullptr;
  udSemaphore *pPresentSemaphore = nullptr;

  // TODO: remove these!
  void(*pResizeCallback)(udViewRef, int, int) = nullptr;
  void(*pPreRenderCallback)(udViewRef, udSceneRef) = nullptr;
  void(*pPostRenderCallback)(udViewRef, udSceneRef) = nullptr;

  udView(const udComponentDesc *pType, udKernel *pKernel, udRCString uid, udInitParams initParams)
    : udComponent(pType, pKernel, uid, initParams) {}
  virtual ~udView();

  static udComponent *Create(const udComponentDesc *pType, udKernel *pKernel, udRCString uid, udInitParams initParams)
  {
    return udNew(udView, pType, pKernel, uid, initParams);
  }
  static udResult InitRender();
};

#endif // UDVIEW_H
