#pragma once
#ifndef EPRENDERSCENE_H
#define EPRENDERSCENE_H
#include "ep/cpp/platform.h"

#include "udRender.h"

#include "ep/cpp/math.h"
#include "ep/cpp/sharedptr.h"
#include "ep/cpp/component/view.h"
#include "ep/cpp/component/resource/udmodel.h"
#include "ep/cpp/component/resource/arraybuffer.h"
#include "ep/cpp/component/resource/shader.h"
#include "ep/cpp/component/resource/model.h"
#include "ep/cpp/component/node/node.h"
#include "ep/cpp/render.h"

#include "hal/render.h"
#include "hal/vertex.h"
#include "hal/texture.h"
#include "hal/shader.h"

#include "renderresource.h"

struct epTexture;

namespace ep {

struct RenderableTexture
{
  size_t uniformIndex;
  RenderTextureRef texture;

  // TODO: fill out with TextureSampler members for texture options
};

struct UDJob
{
  UDJob() : constantBuffers(Alloc, 3) {}
  UDModelRef spModel;
  UDRenderContext context;
  Array<RenderConstantBufferRef, 3> constantBuffers;

  NodeRef udNodePtr; // TODO: Change this so there is no need to pass the scene ptr down
};

struct GeomJob
{
  Double4x4 matrix;

  BlendMode blendMode;
  CullMode cullMode;
  CompareFunc depthCompareFunc;

  epPrimitiveType primType;
  size_t numVertices;
  size_t firstIndex;
  size_t firstVertex;

  RenderShaderProgramRef spProgram;
  SharedArray<RenderShaderProperty> uniforms;

  Array<RenderableTexture, 8> textures;

  Array<RenderArrayRef, 16> arrays;
  Array<epArrayBuffer*, 16> epArrays;
  RenderShaderInputConfigRef spShaderInputConfig;
  RenderArrayRef index;

  // TODO : Stitch this up later
  Array<epVertexRange> vertexRanges;

  // TODO : Hack until we standardise the shader inputs provide by ep.
  RenderShaderProperty viewProjection;
  bool setViewProjectionUniform;

  RenderShaderProperty viewRenderSize;
  bool setViewRenderSize;
  RenderShaderProperty viewDisplaySize;
  bool setViewDisplaySize;
};

class RenderableScene : public RefCounted
{
public:
  Array<UDJob, 4> ud;
  Array<GeomJob, 16> geom;
};
typedef SharedPtr<RenderableScene> RenderableSceneRef;


class RenderableView : public RefCounted
{
public:
  RenderableView(const SharedPtr<Renderer> &spRenderer);

  void CreateResources(); // ** RUN ON THE MAIN THREAD!
  void RenderUD();        // ** RUN ON THE UD THREAD!
  void RenderGPU();       // ** RUN ON THE RENDER THREAD!

  // TODO: REMOVE ME!
  udRenderView *GetRenderView() const { return pRenderView; }
  ArrayBufferRef GetColorBuffer() const { return spColorBuffer; }

  Double4x4 camera;
  Double4x4 projection;

  Float4 clearColor;

  int displayWidth, displayHeight;
  int renderWidth, renderHeight;

  bool pickingEnabled = false;
  udRenderPick udPick = { sizeof(udRenderPick) };
  udRenderHighlight udPickHighlight = { nullptr, 0, 0 };

  RenderableSceneRef spScene = nullptr;

  ViewRef spView = nullptr;

  udRenderEngine *pRenderEngine = nullptr;
  udRenderView *pRenderView = nullptr;
  udRenderOptions options;

  ArrayBufferRef spColorBuffer;
  ArrayBufferRef spDepthBuffer;

  RenderTextureRef spColorTexture;
  RenderTextureRef spDepthTexture;

protected:
  ~RenderableView() override;

  SharedPtr<Renderer> spRenderer;

  // TODO: re-evaluate this
  epSyncPoint *pSyncPoint = nullptr;
};
typedef SharedPtr<RenderableView> RenderableViewRef;


// renderer interface
class Renderer : public RefCounted
{
public:
  Renderer(Kernel *pKernel, int renderThreadCount);
  ~Renderer();

  enum class RenderResourceType
  {
    VertexArray,
    IndexArray,
    Texture
  };

  udRenderEngine *GetRenderEngine() const { return pRenderEngine; }

  void AddUDRenderJob(UniquePtr<RenderableView> job);

  ArrayBufferRef AllocRenderBuffer();
  void ReleaseRenderBuffer(ArrayBufferRef spBuffer) { renderBufferPool.pushBack(spBuffer); }

  RenderShaderInputConfigRef GetShaderInputConfig(Slice<ArrayBufferRef> arrays, const RenderShaderProgramRef &spShaderProgram, Delegate<void(SharedPtr<RefCounted>)> retainShaderInputConfig);
  RenderResourceRef GetRenderBuffer(const ArrayBufferRef &spArrayBuffer, RenderResourceType type);
  RenderResourceRef GetConstantBuffer(const BufferRef &spBuffer);

protected:
  friend class View;
  friend class RenderableView;
  friend class RenderShaderProgram;
  friend class RenderVertexFormat;

  Kernel *pKernel;

  udRenderEngine *pRenderEngine = nullptr;

  udMutex *pUDMutex;
  udSemaphore *pUDSemaphore, *pUDTerminateSemaphore;
  Array<UniquePtr<RenderableView>, 4> udRenderQueue;

  Array<ArrayBufferRef> renderBufferPool;
  int numRenderBuffers = 0;


  static uint32_t UDThreadStart(void *data)
  {
    ((Renderer*)data)->UDThread();
    return 0;
  }
  void UDThread();

private:
  ShaderRef spVertexShader;
  ShaderRef spFragmentShader;
  ArrayBufferRef spQuadVerts;
  ArrayBufferRef spQuadIndices;

  epShaderInputConfig *s_pPosUV = nullptr;
  epArrayBuffer *s_pQuadVB = nullptr;
  epArrayBuffer *s_pQuadIB = nullptr;

  epShader *pVS = nullptr;
  epShader *pPS = nullptr;
  epShaderProgram *s_shader = nullptr;
};

} // namespace ep

#endif // EPRENDERSCENE_H
