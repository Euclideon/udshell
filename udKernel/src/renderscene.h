#pragma once
#ifndef UDRENDERSCENE_H
#define UDRENDERSCENE_H

#include "udRender.h"
#include "udMath.h"

#include "util/udsharedptr.h"
#include "util/udslice.h"
#include "components/view.h"
#include "components/resources/udmodel.h"


struct udTexture;
namespace ud
{

struct UDJob
{
  UDModelRef spModel = nullptr;
  NodeRenderModel renderModel;
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

class RenderScene : public udRefCounted
{
public:
  udFixedSlice<UDJob, 4> ud;
  udFixedSlice<GeomJob> geom;
};
typedef udSharedPtr<RenderScene> RenderSceneRef;


class RenderableView : public udRefCounted
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
typedef udSharedPtr<RenderableView> RenderableViewRef;

} // namespace ud

#endif // UDRENDERSCENE_H
