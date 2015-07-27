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

struct udUDJob
{
  udDouble4x4 matrix;
  udRenderClipArea clipArea;

  udSharedUDModelRef spModel = nullptr;
  udRenderModel renderModel;
};

struct udGeomJob
{
  udDouble4x4 matrix;

  // TODO: has stuff
  // * textures
  // * vertex data
  // * shader
  // * constants
  // * render states
};

class udRenderScene : public udRefCounted
{
public:
  udFixedSlice<udUDJob, 4> ud;
  udFixedSlice<udGeomJob> geom;
};
typedef udSharedPtr<udRenderScene> udRenderSceneRef;


class udRenderableView : public udRefCounted
{
public:
  udRenderableView();
  void RenderUD();
  void RenderGPU() const;

  // TODO: REMOVE ME!
  udRenderView *GetRenderView() const { return pRenderView; }
  void *GetColorBuffer() const { return pColorBuffer; }

  udDouble4x4 view;
  udDouble4x4 projection;

  int displayWidth, displayHeight;
  int renderWidth, renderHeight;

  udRenderSceneRef spScene = nullptr;

  udViewRef spView = nullptr;

  udRenderEngine *pRenderEngine = nullptr;
  udRenderView *pRenderView = nullptr;
  udRenderOptions options;

  void *pColorBuffer = nullptr;
  void *pDepthBuffer = nullptr;

  mutable struct udTexture *pColorTexture = nullptr;
  mutable struct udTexture *pDepthTexture = nullptr;

//  udSemaphore *pRenderSemaphore = nullptr;
//  udSemaphore *pPresentSemaphore = nullptr;

protected:
  ~udRenderableView();
};
typedef udSharedPtr<udRenderableView> udRenderableViewRef;


#endif // UDRENDERSCENE_H
