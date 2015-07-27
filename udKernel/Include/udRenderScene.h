#pragma once
#ifndef UDRENDERSCENE_H
#define UDRENDERSCENE_H

#include "udRender.h"
#include "udSharedPtr.h"
#include "udSlice.h"
#include "udMath.h"
#include "udView.h"
#include "udRender.h"
#include "udModel.h"


struct udTexture;
namespace udKernel
{

struct UDJob
{
  udDouble4x4 matrix;
  udRenderClipArea clipArea;

  SharedUDModelRef spModel = nullptr;
  udRenderModel renderModel;
};

struct GeomJob
{
  udDouble4x4 matrix;

  // TODO: has stuff
  // * textures
  // * vertex data
  // * shader
  // * constants
  // * render states
};

class RenderScene : public RefCounted
{
public:
  udFixedSlice<UDJob, 4> ud;
  udFixedSlice<GeomJob> geom;
};
typedef SharedPtr<RenderScene> RenderSceneRef;


class RenderableView : public RefCounted
{
public:
  RenderableView();
  void RenderUD();
  void RenderGPU() const;

  // TODO: REMOVE ME!
  udRenderView *GetRenderView() const { return pRenderView; }
  void *GetColorBuffer() const { return pColorBuffer; }

  udDouble4x4 view;
  udDouble4x4 projection;

  int displayWidth, displayHeight;
  int renderWidth, renderHeight;

  RenderSceneRef spScene = nullptr;

  ViewRef spView = nullptr;

  udRenderEngine *pRenderEngine = nullptr;
  udRenderView *pRenderView = nullptr;
  udRenderOptions options;

  void *pColorBuffer = nullptr;
  void *pDepthBuffer = nullptr;

  mutable udTexture *pColorTexture = nullptr;
  mutable udTexture *pDepthTexture = nullptr;

//  udSemaphore *pRenderSemaphore = nullptr;
//  udSemaphore *pPresentSemaphore = nullptr;

protected:
  ~RenderableView();
};
typedef SharedPtr<RenderableView> RenderableViewRef;

} // namespace udKernel

#endif // UDRENDERSCENE_H
