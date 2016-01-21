#pragma once
#ifndef EPRENDERSCENE_H
#define EPRENDERSCENE_H

#include "udRender.h"
#include "udChunkedArray.h"

#include "ep/cpp/math.h"
#include "ep/cpp/sharedptr.h"
#include "ep/cpp/component/view.h"
#include "ep/cpp/component/resource/udmodel.h"
#include "ep/cpp/component/resource/arraybuffer.h"
#include "ep/cpp/component/resource/shader.h"
#include "ep/cpp/component/resource/model.h"
#include "ep/cpp/render.h"

#include "hal/vertex.h"
#include "hal/texture.h"
#include "hal/shader.h"

#include "renderresource.h"

struct epTexture;

namespace ep {

struct UDRenderableState : public udRenderModel
{
  Double4x4 matrix;
  udRenderClipArea clipArea;
  UDRenderState::SimpleVoxelDlgt simpleVoxelDel;

  static unsigned VoxelShaderFunc(udRenderModel *pRenderModel, udNodeIndex nodeIndex, udRenderNodeInfo *epUnusedParam(pNodeInfo))
  {
    UDRenderableState *pUDRenderState = static_cast<UDRenderableState*>(pRenderModel);
    udOctree *pOctree = pRenderModel->pOctree;
    uint32_t color = pOctree->pGetNodeColor(pOctree, nodeIndex);
    // TODO : either wrap this in a critical section or create Lua states for each thread
    color = pUDRenderState->simpleVoxelDel(color);
    return color;
  }
};

struct UDJob
{
  UDModelRef spModel = nullptr;
  UDRenderableState renderState;
};

struct GeomJob
{
  Double4x4 matrix;

  Array<RenderTextureRef, 8> textures;
  Array<RenderArrayRef, 16> arrays;
  RenderArrayRef index;

  RenderShaderProgramRef spProgram;
  RenderVertexFormatRef spVertexFormat;

  BlendMode blendMode;
  CullMode cullMode;

  // TODO: has stuff
  // constants
  // render states
};

class RenderableScene : public RefCounted
{
public:
  udRenderFlags renderFlags;
  Array<UDJob, 4> ud;
  Array<GeomJob, 16> geom;
};
typedef SharedPtr<RenderableScene> RenderableSceneRef;


class RenderableView : public RefCounted
{
public:
  RenderableView(Renderer *pRenderer);

  void RenderUD();  // ** RUN ON THE UD THREAD!
  void RenderGPU(); // ** RUN ON THE RENDER THREAD!

  // TODO: REMOVE ME!
  udRenderView *GetRenderView() const { return pRenderView; }
  ArrayBufferRef GetColorBuffer() const { return spColorBuffer; }

  Double4x4 camera;
  Double4x4 projection;

  int displayWidth, displayHeight;
  int renderWidth, renderHeight;

  bool pickingEnabled = false;
  udRenderPick udPick = { sizeof(udRenderPick) };

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

  Renderer *pRenderer;
};
typedef SharedPtr<RenderableView> RenderableViewRef;


// renderer interface
class Renderer
{
public:
  Renderer(kernel::Kernel *pKernel, int renderThreadCount);
  ~Renderer();

  udRenderEngine *GetRenderEngine() const { return pRenderEngine; }

  void AddUDRenderJob(UniquePtr<RenderableView> job);

  ArrayBufferRef AllocRenderBuffer();
  void ReleaseRenderBuffer(ArrayBufferRef spBuffer) { renderBufferPool.pushBack(spBuffer); }

protected:
  friend class View;
  friend class RenderableView;
  friend class RenderShaderProgram;
  friend class RenderVertexFormat;

  kernel::Kernel *pKernel;

  udRenderEngine *pRenderEngine = nullptr;

  udMutex *pUDMutex;
  udSemaphore *pUDSemaphore, *pUDTerminateSemaphore;
  Array<UniquePtr<RenderableView>, 4> udRenderQueue;

  AVLTree<uint32_t, RenderShaderProgram*> shaderPrograms;
  AVLTree<uint32_t, RenderVertexFormat*> vertexFormats;

  Array<ArrayBufferRef> renderBufferPool;
  int numRenderBuffers = 0;

  enum class RenderResourceType
  {
    VertexArray,
    IndexArray,
    Texture,
  };

  RenderResourceRef GetRenderBuffer(const ArrayBufferRef &spArrayBuffer, RenderResourceType type);
  RenderShaderRef GetShader(const ShaderRef &spShader);
  RenderShaderProgramRef GetShaderProgram(const MaterialRef &spShaderProgram);
  RenderVertexFormatRef GetVertexFormat(const RenderShaderProgramRef &spShaderProgram, Slice<VertexArray> arrays);

  void SetRenderstates(MaterialRef spMaterial, RenderShaderProgramRef spProgram);

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

  epFormatDeclaration *s_pPosUV = nullptr;
  epArrayBuffer *s_pQuadVB = nullptr;
  epArrayBuffer *s_pQuadIB = nullptr;
  epShaderProgram *s_shader = nullptr;
};

} // namespace ep

#endif // EPRENDERSCENE_H
