#include "renderscene.h"

#include "hal/render.h"
#include "hal/texture.h"
#include "hal/vertex.h"
#include "hal/shader.h"

// TODO: remove when resource cleanup is implemented
#include "hal/driver.h"

namespace ud
{

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

static udFormatDeclaration *s_pPosUV = nullptr;
static udArrayBuffer *s_pQuadVB = nullptr;
static udArrayBuffer *s_pQuadIB = nullptr;
static udShaderProgram *s_shader = nullptr;

void udRenderScene_InitRender()
{
  // crete a vertex buffer to render the quad to the screen
  udArrayDataFormat format[] = { udVDF_Float2 };
  s_pQuadVB = udVertex_CreateVertexBuffer(format, 1);
  s_pQuadIB = udVertex_CreateIndexBuffer(udVDF_UInt);

  udArrayElement elements[] = {
//    { udVET_Position, 0, 2, udVDF_Float2 },
    { "a_position", format[0], 0 },
  };
  s_pPosUV = udVertex_CreateFormatDeclaration(elements, sizeof(elements)/sizeof(elements[0]));

  struct Vertex
  {
    float x, y;
  } quad[] = {
      { 0, 0 },
      { 1, 0 },
      { 1, 1 },
      { 0, 1 }
  };
  udVertex_SetArrayBufferData(s_pQuadVB, quad, sizeof(quad));

  uint32_t indices[] = { 0, 1, 2, 3 };
  udVertex_SetArrayBufferData(s_pQuadIB, indices, sizeof(indices));

  udShader *pVS = udShader_CreateShader(s_vertexShader, sizeof(s_vertexShader), udST_VertexShader);
  udShader *pPS = udShader_CreateShader(s_blitShader, sizeof(s_blitShader), udST_PixelShader);
  s_shader = udShader_CreateShaderProgram(pVS, pPS);
}

RenderableView::RenderableView()
{
  memset(&options, 0, sizeof(options));
};

RenderableView::~RenderableView()
{
  if (pRenderView)
    udRender_DestroyView(&pRenderView);

  if (pColorBuffer)
    udFree(pColorBuffer);
  if (pDepthBuffer)
    udFree(pDepthBuffer);

  if (pColorTexture)
    udTexture_DestroyTexture(&pColorTexture);
  if (pDepthTexture)
    udTexture_DestroyTexture(&pDepthTexture);
}

void RenderableView::RenderUD()
{
  if (spScene->ud.length)
  {
    size_t size = sizeof(udRenderModel*)*spScene->ud.length;
    udRenderModel **ppRenderModels = (udRenderModel**)alloca(size);

    for (size_t i = 0; i < spScene->ud.length; ++i)
      ppRenderModels[i] = &spScene->ud[i].renderModel;

    // allocate view
    udRender_CreateView(&pRenderView, pRenderEngine, renderWidth, renderHeight);

    udRender_SetMatrixF64(pRenderView, udRMT_Projection, projection.a);
    udRender_SetMatrixF64(pRenderView, udRMT_Camera, view.a);

    // allocate render buffers
    uint32_t colorPitch = renderWidth*sizeof(uint32_t);
    uint32_t depthPitch = renderWidth*sizeof(float);
    pColorBuffer = udAlloc(colorPitch * renderHeight);
    pDepthBuffer = udAlloc(depthPitch * renderHeight);
    udRender_SetTarget(pRenderView, udRTT_Color32, pColorBuffer, colorPitch, 0x80202080);
    udRender_SetTarget(pRenderView, udRTT_Depth32, pDepthBuffer, depthPitch, 0x3F800000);

    // render UD
    udRender_Render(pRenderView, ppRenderModels, (int)spScene->ud.length, &options);
  }
  else
  {
    uint32_t colorPitch = renderWidth*sizeof(uint32_t);
    uint32_t depthPitch = renderWidth*sizeof(float);
    pColorBuffer = udAllocFlags(colorPitch * renderHeight, udAF_Zero);
    pDepthBuffer = udAllocFlags(depthPitch * renderHeight, udAF_Zero);
  }
}

void RenderableView::RenderGPU()
{
//  if (spView->pPreRenderCallback)
//    spView->pPreRenderCallback(spView, spScene);

  if (!pColorTexture)
  {
    // copy the data into the texture
    pColorTexture = udTexture_CreateTexture(udTT_2D, renderWidth, renderHeight, 1, udIF_BGRA8);
    pDepthTexture = udTexture_CreateTexture(udTT_2D, renderWidth, renderHeight, 1, udIF_R_F32);

    // blit the scene to the viewport
    udTexture_SetImageData(pColorTexture, -1, 0, pColorBuffer);
    udTexture_SetImageData(pDepthTexture, -1, 0, pDepthBuffer);
  }

  udShader_SetCurrent(s_shader);

  int u_texture = udShader_FindShaderParameter(s_shader, "u_texture");
  udShader_SetProgramData(0, u_texture, pColorTexture);
  int u_zbuffer = udShader_FindShaderParameter(s_shader, "u_zbuffer");
  udShader_SetProgramData(1, u_zbuffer, pDepthTexture);

  int u_rect = udShader_FindShaderParameter(s_shader, "u_rect");
  udShader_SetProgramData(u_rect, udFloat4::create(-1, 1, 2, -2));
  int u_textureScale = udShader_FindShaderParameter(s_shader, "u_textureScale");
  udShader_SetProgramData(u_textureScale, udFloat4::create(0, 0, 1, 1));

  udGPU_RenderIndices(s_shader, s_pPosUV, &s_pQuadVB, s_pQuadIB, udPT_TriangleFan, 4);

  // TODO: we need to have some sort of resource cleanup list so this can happen when we're ready/automatically
#if UDRENDER_DRIVER == UDDRIVER_QT
  udTexture_DestroyTexture(&pColorTexture);
  udTexture_DestroyTexture(&pDepthTexture);
  pColorTexture = nullptr;
  pDepthTexture = nullptr;
#endif

/*
  if (pPostRenderCallback)
    pPostRenderCallback(ViewRef(this), spScene);
*/
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
  pUDMutex = udCreateMutex();
  pUDSemaphore = udCreateSemaphore(65536, 0);
  pUDTerminateSemaphore = udCreateSemaphore(1, 0);
  udCreateThread(UDThreadStart, this);
}
Renderer::~Renderer()
{
  // terminate UD thread
  udIncrementSemaphore(pUDSemaphore);
  udWaitSemaphore(pUDTerminateSemaphore);

  // clean up synchronisation tools
  udDestroyMutex(&pUDMutex);
  udDestroySemaphore(&pUDSemaphore);
  udDestroySemaphore(&pUDTerminateSemaphore);

  // destroy render engine
  udRender_Destroy(&pRenderEngine);
}

void Renderer::AddUDRenderJob(udUniquePtr<RenderableView> job)
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
    JobDone(const udUniquePtr<RenderableView> &job) : job(job) {}

    udUniquePtr<RenderableView> job;

    void FinishJob(Kernel *pKernel)
    {
      ViewRef spView = job->spView;
      spView->SetLatestFrame(job);
      delete this;
    }
  };

  while (1)
  {
    udWaitSemaphore(pUDSemaphore);

    udLockMutex(pUDMutex);
    if (udRenderQueue.length == 0)
    {
      udReleaseMutex(pUDMutex);
      break;
    }
    udUniquePtr<RenderableView> job = udRenderQueue.popFront();
    udReleaseMutex(pUDMutex);

    job->RenderUD();

    JobDone *done = new JobDone(job);
    pKernel->DispatchToMainThread(MakeDelegate(done, &JobDone::FinishJob));
  }

  udIncrementSemaphore(pUDTerminateSemaphore);
}

} // namespace ud
