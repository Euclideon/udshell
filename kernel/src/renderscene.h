#pragma once
#ifndef EPRENDERSCENE_H
#define EPRENDERSCENE_H

#include "udRender.h"
#include "udChunkedArray.h"

#include "ep/cpp/math.h"
#include "ep/cpp/sharedptr.h"
#include "components/view.h"
#include "components/resources/udmodel.h"
#include "components/resources/array.h"
#include "components/resources/shader.h"
#include "components/resources/model.h"
#include "hal/vertex.h"
#include "hal/texture.h"
#include "hal/shader.h"
#include "renderresource.h"

struct epTexture;

namespace ep
{

struct UDJob
{
  UDModelRef spModel = nullptr;
  UDRenderState renderState;
};

struct GeomJob
{
  Double4x4 matrix;

  uint32_t numTextures, numArrays;
  RenderTextureRef textures[8];
  RenderArrayRef arrays[16];
  RenderArrayRef index;

  RenderShaderProgramRef spProgram;
  RenderVertexFormatRef spVertexFormat;

  BlendMode blendMode;
  CullMode cullMode;

  // TODO: has stuff
  // constants
  // render states
};

class RenderScene : public RefCounted
{
public:
  // ud thread
  Array<UDJob, 4> ud;

  // render thread
  Array<GeomJob, 16> geom;

protected:
  ~RenderScene()
  {
  }
};
typedef SharedPtr<RenderScene> RenderSceneRef;


class RenderableView : public RefCounted
{
public:
  RenderableView();

  void RenderUD();  // ** RUN ON THE UD THREAD!
  void RenderGPU(); // ** RUN ON THE RENDER THREAD!

  // TODO: REMOVE ME!
  udRenderView *GetRenderView() const { return pRenderView; }
  void *GetColorBuffer() const { return pColorBuffer; }

  Double4x4 camera;
  Double4x4 projection;

  int displayWidth, displayHeight;
  int renderWidth, renderHeight;

  bool pickingEnabled = false;
  udRenderPick udPick = { sizeof(udRenderPick) };

  RenderSceneRef spScene = nullptr;

  ViewRef spView = nullptr;

  udRenderEngine *pRenderEngine = nullptr;
  udRenderView *pRenderView = nullptr;
  udRenderOptions options;

  void *pColorBuffer = nullptr;
  void *pDepthBuffer = nullptr;

  epTexture *pColorTexture = nullptr;
  epTexture *pDepthTexture = nullptr;

protected:
  ~RenderableView() override;
};
typedef SharedPtr<RenderableView> RenderableViewRef;


// renderer interface
class Renderer
{
public:
  Renderer(Kernel *pKernel, int renderThreadCount);
  ~Renderer();

  udRenderEngine *GetRenderEngine() const { return pRenderEngine; }

  void AddUDRenderJob(UniquePtr<RenderableView> job);

protected:
  friend class View;
  friend class RenderShaderProgram;
  friend class RenderVertexFormat;

  static uint32_t UDThreadStart(void *data)
  {
    ((Renderer*)data)->UDThread();
    return 0;
  }
  void UDThread();

  Kernel *pKernel;

  udRenderEngine *pRenderEngine = nullptr;

  udMutex *pUDMutex;
  udSemaphore *pUDSemaphore, *pUDTerminateSemaphore;
  Array<UniquePtr<RenderableView>, 4> udRenderQueue;

  AVLTree<uint32_t, RenderShaderProgram*> shaderPrograms;
  AVLTree<uint32_t, RenderVertexFormat*> vertexFormats;
};

} // namespace ep

#endif // EPRENDERSCENE_H
