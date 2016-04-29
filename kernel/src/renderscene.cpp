#include "renderscene.h"
#include "renderresource.h"
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

#include "ep/cpp/kernel.h"


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

  if (pSyncPoint)
    epGPU_DestroySyncPoint(&pSyncPoint);
}

void RenderableView::CreateResources()
{
  if (!spColorTexture)
  {
    spColorTexture = shared_pointer_cast<RenderTexture>(spRenderer->GetRenderBuffer(spColorBuffer, Renderer::RenderResourceType::Texture));
    spDepthTexture = shared_pointer_cast<RenderTexture>(spRenderer->GetRenderBuffer(spDepthBuffer, Renderer::RenderResourceType::Texture));
    // TODO: use this to ensure the resources have been fully uploaded in the GPU before rendering - re-think this whole thing
    pSyncPoint = epGPU_CreateSyncPoint();
  }
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

  if (spColorBuffer && spColorTexture)
  {
    // TODO: wait for the sync point to be processed - re-think this whole thing
    epGPU_WaitSync(&pSyncPoint);

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

  // render geometry
  Double4x4 vp = Mul(projection, Inverse(camera));
  for (auto &job : spScene->geom)
  {
    Double4x4 wvp = Mul(vp, job.matrix);

    // TODO : Textures
    // TODO : Renderstate

    job.spProgram->Use();
    job.spProgram->setUniform(job.viewProjection.index, Float4x4::create(wvp));
    for (size_t i = 0; i < job.uniforms.length; ++i)
      job.spProgram->setUniform(job.uniforms[i].index, job.uniforms[i].data);

    if (job.index)
      epGPU_RenderIndices(job.spProgram->pProgram, job.spShaderInputConfig->pConfig, job.epArrays.ptr, job.index->pArray,
                          job.primType, job.numVertices, job.firstIndex, job.firstVertex);
    else
      epGPU_RenderRanges(job.spProgram->pProgram, job.spShaderInputConfig->pConfig, job.epArrays.ptr, job.primType,
                         job.vertexRanges.ptr, job.vertexRanges.length);
  }

//  if (pPostRenderCallback)
//    pPostRenderCallback(ViewRef(this), spScene);
}

Renderer::Renderer(Kernel *pKernel, int renderThreadCount)
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
  // TODO: Error handling
  pUDMutex = udCreateMutex();
  pUDSemaphore = udCreateSemaphore(65536, 0);
  pUDTerminateSemaphore = udCreateSemaphore(1, 0);
  udCreateThread(UDThreadStart, this);

  // GPU init
  // create a vertex buffer to render the quad to the screen
  epArrayDataFormat format[] = { epVDF_Float2 };
  s_pQuadVB = epVertex_CreateVertexBuffer(format, 1);
  EPASSERT_THROW(s_pQuadVB, epR_Failure, "Failed to create vertex buffer");
  epscope(fail) { epVertex_DestroyArrayBuffer(&s_pQuadVB); };

  s_pQuadIB = epVertex_CreateIndexBuffer(epVDF_UInt);
  EPASSERT_THROW(s_pQuadIB, epR_Failure, "Failed to create index buffer");
  epscope(fail) { epVertex_DestroyArrayBuffer(&s_pQuadIB); };

  epArrayElement elements[] =
  {
    { "a_position", format[0], 0, 0, 8 }
  };

  pVS = epShader_CreateShader(s_vertexShader, sizeof(s_vertexShader), epST_VertexShader);
  EPASSERT_THROW(pVS, epR_Failure, "Failed to create vertex shader");
  epscope(fail) { epShader_DestroyShader(&pVS); };

  pPS = epShader_CreateShader(s_blitShader, sizeof(s_blitShader), epST_PixelShader);
  EPASSERT_THROW(pPS, epR_Failure, "Failed to create pixel shader");
  epscope(fail) { epShader_DestroyShader(&pPS); };

  epShader *shaders[] = { pVS, pPS };
  s_shader = epShader_CreateShaderProgram(shaders, 2);
  EPASSERT_THROW(s_shader, epR_Failure, "Failed to create shader program");
  epscope(fail) { epShader_DestroyShaderProgram(&s_shader); };

  s_pPosUV = epVertex_CreateShaderInputConfig(elements, sizeof(elements)/sizeof(elements[0]), s_shader);
  EPASSERT_THROW(s_pPosUV, epR_Failure, "Failed to create shader input configuration");

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
}

Renderer::~Renderer()
{
  // GPU destruction
  // TODO: Add in calls to epShader_Destroy when implemented.
  epShader_DestroyShaderProgram(&s_shader);
  epShader_DestroyShader(&pPS);
  epShader_DestroyShader(&pVS);
  epVertex_DestroyShaderInputConfig(&s_pPosUV);
  epVertex_DestroyArrayBuffer(&s_pQuadIB);
  epVertex_DestroyArrayBuffer(&s_pQuadVB);

  // terminate UD thread
  udIncrementSemaphore(pUDSemaphore);
  udWaitSemaphore(pUDTerminateSemaphore);

  // clean up synchronisation tools
  udDestroyMutex(&pUDMutex);
  udDestroySemaphore(&pUDSemaphore);
  udDestroySemaphore(&pUDTerminateSemaphore);

  udStreamerStatus streamerStatus = { 0 };
  do
  {
    udOctree_Update(&streamerStatus);
  } while (streamerStatus.active);

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

    pArrayBuffer->spCachedRenderData = spRenderBuffer;
  }
  return spRenderBuffer;
}

RenderShaderInputConfigRef Renderer::GetShaderInputConfig(Slice<ArrayBufferRef> arrays, const RenderShaderProgramRef &spShaderProgram, Delegate<void(SharedPtr<RefCounted>)> retainShaderInputConfig)
{
  RenderShaderInputConfigRef spShaderInputConfig = SharedPtr<RenderShaderInputConfig>::create(this, arrays, spShaderProgram);
  retainShaderInputConfig(spShaderInputConfig);

  return spShaderInputConfig;
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

    void FinishJob()
    {
      ViewRef spView = job->spView;
      if (spView)
      {
        job->CreateResources();
        // NOTE: we need to clear this pointer here to prevent circular referencing!
        job->spView = nullptr;
        spView->GetImpl<ViewImpl>()->SetLatestFrame(job);
      }
      epDelete(this);
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

    JobDone *done = epNew(JobDone, job);
    pKernel->DispatchToMainThread(MakeDelegate(done, &JobDone::FinishJob));
  }

  udIncrementSemaphore(pUDTerminateSemaphore);
}

} // namespace ep
