#pragma once
#ifndef UDVIEW_H
#define UDVIEW_H

#include "udComponent.h"
#include "udInput.h"
#include "udRender.h"
#include "udMath.h"

class udScene;
class udCamera;

class udRenderableView
{
public:
  class udRenderScene *pScene; // ref counted pointer to the most recent renderable scene...

  udDouble4x4 view;
  udDouble4x4 projection;

protected:
  int refCount;
};

class udView : public udComponent
{
public:
  virtual udResult InputEvent(const udInputEvent &ev);
  virtual udResult Resize(int width, int height);

  udResult Render(); // TODO: REMOVE ME
  udRenderView *GetUDRenderView() { return pRenderView; } // TODO: REMOVE ME

  udRenderView *GetRenderView(); // TODO: return an immutable object that the render thread can render

  void SetScene(udScene *pScene) { this->pScene = pScene; }
  void SetCamera(udCamera *pCamera) { this->pCamera = pCamera; }
  udScene *GetScene() const { return pScene; }
  udCamera *GetCamera() const { return pCamera; }

  void GetDimensions(int *pWidth, int *pHeight) const;
  void GetRenderDimensions(int *pWidth, int *pHeight) const;
  float GetAspectRatio() const { return (float)displayWidth / (float)displayHeight; }

  void* GetColorBuffer() const { return pColorBuffer; }
  void* GetDepthBuffer() const { return pDepthBuffer; }

  // TODO: remove these!
  void RegisterResizeCallback(void (*pCallback)(udView *pView, int w, int h)) { pResizeCallback = pCallback; }
  void RegisterPreRenderCallback(void(*pCallback)(udView *pView, udScene *pScene)) { pPreRenderCallback = pCallback; }
  void RegisterPostRenderCallback(void(*pCallback)(udView *pView, udScene *pScene)) { pPostRenderCallback = pCallback; }

  static const udComponentDesc descriptor;

protected:
  udScene *pScene = nullptr;
  udCamera *pCamera = nullptr;

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
  void(*pResizeCallback)(udView*, int, int) = nullptr;
  void(*pPreRenderCallback)(udView*, udScene*) = nullptr;
  void(*pPostRenderCallback)(udView*, udScene*) = nullptr;

  udView(const udComponentDesc *pType, udKernel *pKernel, udRCString uid, udInitParams initParams)
    : udComponent(pType, pKernel, uid, initParams) {}
  virtual ~udView() {}

  static udComponent *Create(const udComponentDesc *pType, udKernel *pKernel, udRCString uid, udInitParams initParams)
  {
    return udNew(udView, pType, pKernel, uid, initParams);
  }
};

#endif // UDVIEW_H
