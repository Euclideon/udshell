#pragma once
#ifndef UDRENDERSCENE_H
#define UDRENDERSCENE_H

#include "udRender.h"
#include "udSharedPtr.h"
#include "udSlice.h"
#include "udMath.h"
#include "udView.h"

struct udUDJob
{
  udDouble4x4 matrix;
  udOctree *pOctree;
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

struct udRenderScene
{
  udFixedSlice<udUDJob, 3> ud;
  udFixedSlice<udGeomJob> geom;
};

struct udRenderableView
{
  ~udRenderableView();

  void RenderUD();
  void RenderGPU() const;

  // TODO: REMOVE ME!
  udRenderView *GetRenderView() const { return pRenderView; }
  void *GetColorBuffer() const { return pColorBuffer; }

  udDouble4x4 view;
  udDouble4x4 projection;

  int displayWidth, displayHeight;
  int renderWidth, renderHeight;

  udSharedPtr<const udRenderScene> scene;

  udViewRef spView;

  udRenderEngine *pRenderEngine = nullptr;
  udRenderView *pRenderView = nullptr;
  udRenderOptions options;

  void *pColorBuffer = nullptr;
  void *pDepthBuffer = nullptr;

  mutable struct udTexture *pColorTexture = nullptr;
  mutable struct udTexture *pDepthTexture = nullptr;

//  udSemaphore *pRenderSemaphore = nullptr;
//  udSemaphore *pPresentSemaphore = nullptr;
};


#endif // UDRENDERSCENE_H
