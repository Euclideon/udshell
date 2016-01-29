#include "renderscene.h"

#include "hal/render.h"
#include "hal/texture.h"
#include "hal/vertex.h"
#include "hal/shader.h"

#include "ep/cpp/component/resource/arraybuffer.h"
#include "ep/cpp/component/resource/shader.h"

#include "components/viewimpl.h"
#include "components/resources/arraybufferimpl.h"
#include "components/resources/shaderimpl.h"
#include "components/resources/materialimpl.h"

// TODO: remove when resource cleanup is implemented
#include "hal/driver.h"

namespace ep {

// shaders for blitting
const char s_vertexShader[] =
"attribute vec2 a_position;\n"
"varying vec2 v_texcoord;\n"
"varying vec2 v_texcoord2;\n"
"uniform vec4 u_rect;\n"
"uniform vec4 u_textureScale;\n"
"uniform vec4 u_textureScale2;\n"
"void main()\n"
"{\n"
"  v_texcoord = u_textureScale.xy + a_position*u_textureScale.zw;\n"
"  v_texcoord2 = u_textureScale2.xy + a_position*u_textureScale2.zw;\n"
"  vec2 pos = u_rect.xy + a_position*u_rect.zw;\n"
"  gl_Position = vec4(pos.x, pos.y, 1, 1);\n"
"}\n";

const char s_blitShader[] =
#if defined(USE_GLES)
"#extension GL_EXT_frag_depth : require\n"
"precision mediump float;\n"
#endif
"varying vec2 v_texcoord;\n"
"uniform sampler2D u_texture;\n"
"uniform sampler2D u_zbuffer;\n"
"void main()\n"
"{\n"
"  float depthSample = texture2D(u_zbuffer, v_texcoord).x;\n"
"  float z = (gl_DepthRange.diff*depthSample + gl_DepthRange.near + gl_DepthRange.far) / 2.0;\n"
#if defined(USE_GLES)
"  gl_FragDepthEXT = z;\n"
#else
"  gl_FragDepth = z;\n"
#endif
"  gl_FragColor = texture2D(u_texture, v_texcoord);\n"
"}\n";

#include "kernel.h"


RenderableView::RenderableView(const SharedPtr<Renderer> &spRenderer)
  : spRenderer(spRenderer)
{
  memset(&options, 0, sizeof(options));
  options.size = sizeof(udRenderOptions);
};

RenderableView::~RenderableView()
{
  if (pRenderView)
    udRender_DestroyView(&pRenderView);

  if(spColorBuffer)
    spRenderer->ReleaseRenderBuffer(spColorBuffer);
  if(spDepthBuffer)
    spRenderer->ReleaseRenderBuffer(spDepthBuffer);
}

void RenderableView::RenderUD()
{
  Slice<uint32_t> colorBuffer = spColorBuffer->Map<uint32_t>();
  epscope(exit) { spColorBuffer->Unmap(); };
  Slice<float> depthBuffer = spDepthBuffer->Map<float>();
  epscope(exit) { spDepthBuffer->Unmap(); };

  uint32_t colorPitch = renderWidth*sizeof(uint32_t);
  uint32_t depthPitch = renderWidth*sizeof(float);

  if (spScene->ud.length)
  {
    size_t size = sizeof(udRenderModel*)*spScene->ud.length;
    udRenderModel **ppRenderModels = (udRenderModel**)alloca(size);

    for (size_t i = 0; i < spScene->ud.length; ++i)
      ppRenderModels[i] = &spScene->ud[i].renderState;

    // allocate view
    udRender_CreateView(&pRenderView, pRenderEngine, renderWidth, renderHeight);

    udRender_SetMatrixF64(pRenderView, udRMT_Projection, projection.a);
    udRender_SetMatrixF64(pRenderView, udRMT_Camera, camera.a);

    options.flags = spScene->renderFlags;

    if (pickingEnabled)
      options.pick = &udPick;

    // allocate render buffers
    udRender_SetTarget(pRenderView, udRTT_Color32, colorBuffer.ptr, colorPitch, 0xFF202080);
    udRender_SetTarget(pRenderView, udRTT_Depth32, depthBuffer.ptr, depthPitch, 0x3F800000);

    // render UD
    udRender_Render(pRenderView, ppRenderModels, (int)spScene->ud.length, &options);
  }
  else
  {
    memset(colorBuffer.ptr, 0, colorPitch * renderHeight);
    memset(depthBuffer.ptr, 0, colorPitch * renderHeight);
  }
}

void RenderableView::RenderGPU()
{
//  if (spView->pPreRenderCallback)
//    spView->pPreRenderCallback(spView, spScene);

  if (spColorBuffer)
  {
    if (!spColorTexture)
    {
      spColorTexture = spRenderer->GetRenderBuffer(spColorBuffer, Renderer::RenderResourceType::Texture);
      spDepthTexture = spRenderer->GetRenderBuffer(spDepthBuffer, Renderer::RenderResourceType::Texture);
    }

    epShader_SetCurrent(spRenderer->s_shader);

    int u_texture = epShader_FindShaderParameter(spRenderer->s_shader, "u_texture");
    epShader_SetProgramData(0, u_texture, spColorTexture->pTexture);
    int u_zbuffer = epShader_FindShaderParameter(spRenderer->s_shader, "u_zbuffer");
    epShader_SetProgramData(1, u_zbuffer, spDepthTexture->pTexture);

    int u_rect = epShader_FindShaderParameter(spRenderer->s_shader, "u_rect");
    epShader_SetProgramData(u_rect, Float4::create(-1, 1, 2, -2));
    int u_textureScale = epShader_FindShaderParameter(spRenderer->s_shader, "u_textureScale");
    epShader_SetProgramData(u_textureScale, Float4::create(0, 0, 1, 1));

    epGPU_RenderIndices(spRenderer->s_shader, spRenderer->s_pPosUV, &spRenderer->s_pQuadVB, spRenderer->s_pQuadIB, epPT_TriangleFan, 4);
  }
  else
  {
    // clear the backbuffer
    ep::Double4 color = { 0, 0, 1, 0 };
    epGPU_Clear(&color.x, -1.0, 0);
  }

//  // render geometry
//  for (auto &job : this->spScene->geom)
//  {
//    // ...render polygons...
//  }

//  if (pPostRenderCallback)
//    pPostRenderCallback(ViewRef(this), spScene);
}

Renderer::Renderer(kernel::Kernel *pKernel, int renderThreadCount)
  : pKernel(pKernel)
{
  // TODO: Remove this once webview is properly integrated
  const int streamerBuffer = 550*1048576; // TODO : make this an optional command string input
  if (renderThreadCount != -1)
  {
    udRender_Create(&pRenderEngine, renderThreadCount);
    udOctree_Init(streamerBuffer);
  }

  // create UD thread
  pUDMutex = udCreateMutex();
  pUDSemaphore = udCreateSemaphore(65536, 0);
  pUDTerminateSemaphore = udCreateSemaphore(1, 0);
  udCreateThread(UDThreadStart, this);

  // GPU init
  // create a vertex buffer to render the quad to the screen
  epArrayDataFormat format[] = { epVDF_Float2 };
  s_pQuadVB = epVertex_CreateVertexBuffer(format, 1);
  if (!s_pQuadVB)
    EPTHROW(epR_Failure, "TODO: better error");

  s_pQuadIB = epVertex_CreateIndexBuffer(epVDF_UInt);
  if (!s_pQuadIB)
  {
    epVertex_DestroyArrayBuffer(&s_pQuadVB);
    EPTHROW(epR_Failure, "TODO: better error");
  }

  epArrayElement elements[] = {
    //    { udVET_Position, 0, 2, epVDF_Float2 },
    { "a_position", format[0], 0 },
  };

  s_pPosUV = epVertex_CreateFormatDeclaration(elements, sizeof(elements)/sizeof(elements[0]));
  if (!s_pPosUV)
  {
    epVertex_DestroyArrayBuffer(&s_pQuadIB);
    epVertex_DestroyArrayBuffer(&s_pQuadVB);
    EPTHROW(epR_Failure, "TODO: better error");
  }
/*
  using F2 = std::tuple<float, float>;
  using Vertex = std::tuple<const F2>;
  const Vertex quad[4] = {
    { F2{ 0.f, 0.f } },
    { F2{ 1.f, 0.f } },
    { F2{ 1.f, 1.f } },
    { F2{ 0.f, 1.f } }
  };
  uint32_t indices[4] = { 0, 1, 2, 3 };

  spQuadVerts = pKernel->CreateComponent<ArrayBuffer>({ { "name", "quad_vb" } });
  spQuadIndices = pKernel->CreateComponent<ArrayBuffer>({ { "name", "quad_ib" } });
  spQuadVerts->Allocate<Vertex>(4);
  auto buffer = spQuadVerts->Map<Vertex>();
  spQuadVerts->Unmap();
  spQuadIndices->AllocateFromData(Slice<const uint32_t>(indices, 4));
*/
  struct Vertex
  {
    float x, y;
  } quad[] = {
    { 0, 0 },
    { 1, 0 },
    { 1, 1 },
    { 0, 1 }
  };

  epVertex_SetArrayBufferData(s_pQuadVB, quad, sizeof(quad));

  uint32_t indices[] = { 0, 1, 2, 3 };
  epVertex_SetArrayBufferData(s_pQuadIB, indices, sizeof(indices));

  epShader *pVS = epShader_CreateShader(s_vertexShader, sizeof(s_vertexShader), epST_VertexShader);
  if (!pVS)
  {
    epVertex_DestroyFormatDeclaration(&s_pPosUV);
    epVertex_DestroyArrayBuffer(&s_pQuadIB);
    epVertex_DestroyArrayBuffer(&s_pQuadVB);
    EPTHROW(epR_Failure, "TODO: better error");
  }

  epShader *pPS = epShader_CreateShader(s_blitShader, sizeof(s_blitShader), epST_PixelShader);
  if (!pPS)
  {
    // TODO: Add in calls to epShader_Destroy when implemented.
    //epShader_Destroy(&pVS)
    epVertex_DestroyFormatDeclaration(&s_pPosUV);
    epVertex_DestroyArrayBuffer(&s_pQuadIB);
    epVertex_DestroyArrayBuffer(&s_pQuadVB);
    EPTHROW(epR_Failure, "TODO: better error");
  }

  s_shader = epShader_CreateShaderProgram(pVS, pPS);
  if (!s_shader)
  {
    // TODO: Add in calls to epShader_Destroy when implemented.
    //epShader_Destroy(&pPS)
    //epShader_Destroy(&pVS)
    epVertex_DestroyFormatDeclaration(&s_pPosUV);
    epVertex_DestroyArrayBuffer(&s_pQuadIB);
    epVertex_DestroyArrayBuffer(&s_pQuadVB);
    EPTHROW(epR_Failure, "TODO: better error");
  }
}

Renderer::~Renderer()
{
  // GPU destruction
  // TODO: Add in calls to epShader_Destroy when implemented.
  //epShader_DestroyShaderProgram(s_shader);
  //epShader_Destroy(&pPS);
  //epShader_Destroy(&pVS);
  epVertex_DestroyFormatDeclaration(&s_pPosUV);
  epVertex_DestroyArrayBuffer(&s_pQuadIB);
  epVertex_DestroyArrayBuffer(&s_pQuadVB);

  // terminate UD thread
  udIncrementSemaphore(pUDSemaphore);
  udWaitSemaphore(pUDTerminateSemaphore);

  // clean up synchronisation tools
  udDestroyMutex(&pUDMutex);
  udDestroySemaphore(&pUDSemaphore);
  udDestroySemaphore(&pUDTerminateSemaphore);

  udOctree_Shutdown();

  // destroy render engine
  udRender_Destroy(&pRenderEngine);
}

RenderResourceRef Renderer::GetRenderBuffer(const ArrayBufferRef &spArrayBuffer, RenderResourceType type)
{
  ArrayBufferImpl *pArrayBuffer = spArrayBuffer->GetImpl<ArrayBufferImpl>();

  RenderResourceRef spRenderBuffer = shared_pointer_cast<RenderResource>(pArrayBuffer->spCachedRenderData);
  if (!spRenderBuffer)
  {
    if (type == RenderResourceType::Texture)
    {
      // TODO: we need to find texture type from the dimensions of the array buffer
      //       ... or we need 'type' to ask for the particular texture type that the shader expects
      spRenderBuffer = RenderTextureRef::create(this, spArrayBuffer);
    }
    else
      spRenderBuffer = RenderArrayRef::create(this, spArrayBuffer, type == RenderResourceType::IndexArray ? ArrayUsage::IndexData : ArrayUsage::VertexData);

//    pArrayBuffer->spCachedRenderData = spRenderBuffer; // TODO: if we cache this, it will be destroyed from the main thread, and renderables must destroy from the render thread! >_<
  }
  return spRenderBuffer;
}
RenderShaderRef Renderer::GetShader(const ShaderRef &spShader)
{
  ShaderImpl *pShader = spShader->GetImpl<ShaderImpl>();

  RenderShaderRef spRenderShader = shared_pointer_cast<RenderShader>(pShader->spCachedShader);
  /*
  if (!spShader)
  {
    RenderShader *pShader = new RenderShader(this, ShaderRef(pInstance), (epShaderType)type);
    if (!pShader->pShader)
    {
      delete pShader;
      spRenderShader = nullptr;
    }
    else
      spRenderShader = RenderShaderRef(pShader);
  }
  */
  return spRenderShader;
}
RenderShaderProgramRef Renderer::GetShaderProgram(const MaterialRef &spShaderProgram)
{
//  MaterialImpl *pMaterial = spShaderProgram->GetImpl<MaterialImpl>();
  /*
  RenderResourceRef spShaderProgram = shared_pointer_cast<RenderResource>(pMaterial->spCachedShaderProgram);
  if (!spShaderProgram)
  {
    RenderShaderRef spVS = shaders[0] ? shaders[0]->GetImpl<ShaderImpl>()->GetRenderShader(epST_VertexShader) : nullptr;
    RenderShaderRef spPS = shaders[1] ? shaders[1]->GetImpl<ShaderImpl>()->GetRenderShader(epST_PixelShader) : nullptr;
    if (spVS && spPS)
    {
      // TODO: check if this program already exists in `pRenderer->shaderPrograms`

      RenderShaderProgram *pProgram = new RenderShaderProgram(this, spVS, spPS);
      if (!pProgram->pProgram)
      {
        delete pProgram;
        spRenderProgram = nullptr;
      }
      else
        spRenderProgram = RenderShaderProgramRef(pProgram);

      // populate the material with properties from the shader...
      // TODO...

      pMaterial->spCachedShaderProgram = spShaderProgram;
    }
  }
  */
  return spShaderProgram;
}
RenderVertexFormatRef Renderer::GetVertexFormat(const RenderShaderProgramRef &spShaderProgram, Slice<VertexArray> arrays)
{
  SharedPtr<RefCounted> spRenderVertexFormat;
  if (!spRenderVertexFormat)
  {
    EPASSERT(false, "TODO");

    // get the attributes from the material

    // create a descriptor that maps vertex arrays to shader attributes

    //    VertexFormatDescriptor *pFormat = new VertexFormatDescriptor(this, spArrays);
    //    spRenderVertexFormat = VertexFormatDescriptorRef(pFormat);
  }
  return spRenderVertexFormat;
}

void Renderer::SetRenderstates(MaterialRef spMaterial, RenderShaderProgramRef spProgram)
{
  MaterialImpl *pMat = spMaterial->GetImpl<MaterialImpl>();
  auto &properties = pMat->MaterialProperties();
  size_t numUniforms = spProgram->numUniforms();
  for (size_t i = 0; i < numUniforms; ++i)
  {
    const Float4 *pVal = properties.Get(spProgram->getUniformName(i));
    if (pVal)
      spProgram->setUniform((int)i, *pVal);
  }
}

ArrayBufferRef Renderer::AllocRenderBuffer()
{
  if (renderBufferPool.empty())
    return pKernel->CreateComponent<ArrayBuffer>({ { "name", SharedString::format("udrenderbuffer{0}", numRenderBuffers++) } });
  return renderBufferPool.popBack();
}

void Renderer::AddUDRenderJob(UniquePtr<RenderableView> job)
{
  udLockMutex(pUDMutex);

  for (size_t i = 0; i < udRenderQueue.length; ++i)
  {
    if (udRenderQueue[i]->spView == job->spView)
    {
      // replace queued job with latest frame
      udRenderQueue[i] = job;
      udReleaseMutex(pUDMutex);
      return;
    }
  }

  // add new job
  udRenderQueue.concat(job);
  udReleaseMutex(pUDMutex);
  udIncrementSemaphore(pUDSemaphore);
}

void Renderer::UDThread()
{
  struct JobDone
  {
    JobDone(UniquePtr<RenderableView> &job) : job(job) {}

    UniquePtr<RenderableView> job;

    void FinishJob(ep::Kernel *_pKernel)
    {
      ViewRef spView = job->spView;
      spView->GetImpl<ViewImpl>()->SetLatestFrame(job);
      delete this;
    }
  };

  while (1)
  {
    int result = udWaitSemaphore(pUDSemaphore);
    if (result == -1)
      return;

    udLockMutex(pUDMutex);
    if (udRenderQueue.length == 0)
    {
      udReleaseMutex(pUDMutex);
      break;
    }
    UniquePtr<RenderableView> job = udRenderQueue.popFront();
    udReleaseMutex(pUDMutex);

    try {
      job->RenderUD();
    } catch(...) {
      // this frame failed to render...
      // TODO: message? produce an default/invalid/failed image?
    }

    JobDone *done = new JobDone(job);
    pKernel->DispatchToMainThread(MakeDelegate(done, &JobDone::FinishJob));
  }

  udIncrementSemaphore(pUDTerminateSemaphore);
}

} // namespace ep
