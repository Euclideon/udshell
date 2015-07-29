#include "udPlatform.h"
#include "udPlatformUtil.h"
#include "udRender.h"
#include "udMath.h"

#include "hal/render.h"
#include "hal/texture.h"
#include "hal/vertex.h"
#include "hal/shader.h"
#include "hal/input.h"
#include "viewer_internal.h"

#include "components/nodes/camera.h"


void udDebugFont_Init();
void udDebugFont_Deinit();

#if 0

struct MessageHandler
{
  char name[64];
  udViewer_MessageCallback *pCallback;
  void *pUserData;
};

MessageHandler *s_pMessageHandlers = nullptr;
size_t s_numMessageHandlers = 0, s_numMessageHandlersAllocated = 0;


UDTHREADLOCAL udViewerInstance *s_pCurrentInstance;

udVertexDeclaration *s_pPosUV;
udVertexBuffer *s_pQuadVB;
udShaderProgram *s_shader;


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


//-----------------------------------------------------
// API functions

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udViewer_Init(const udViewerInitParams &initParams)
{
  udViewerInstance *pInstance = udViewerDriver_CreateInstance();
  s_pCurrentInstance = pInstance;

  pInstance->initParams = initParams;
  pInstance->data.pUserData = initParams.pUserData;

  if (initParams.renderThreadCount >= 0)
    udRender_Create(&pInstance->data.pRenderEngine, initParams.renderThreadCount);

  udViewerDriver_Init(pInstance);

  udInput_Init();
  udInput_LockMouseOnButtons(1 << udMC_LeftButton);

  // crete a vertex buffer to render the quad to the screen
  udVertexElement elements[] = {
    { udVET_Position, 0, 2, udVDF_Float2 },
  };
  s_pPosUV = udVertex_CreateVertexDeclaration(elements, sizeof(elements)/sizeof(elements[0]));
  s_pQuadVB = udVertex_CreateVertexBuffer(s_pPosUV);

  struct Vertex
  {
    float x, y;
  } quad[] = {
    { 0,0 },
    { 1,0 },
    { 1,1 },
    { 0,1 }
  };
  udVertex_SetVertexBufferData(s_pQuadVB, quad, sizeof(quad));

  udShader *pVS = udShader_CreateShader(s_vertexShader, sizeof(s_vertexShader), udST_VertexShader);
  udShader *pPS = udShader_CreateShader(s_blitShader, sizeof(s_blitShader), udST_PixelShader);
  s_shader = udShader_CreateShaderProgram(pVS, pPS);

  udDebugFont_Init();
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udViewer_Deinit()
{
  udRender_DestroyView(&s_pCurrentInstance->data.pRenderView);

  udViewerDriver_Deinit(s_pCurrentInstance);

  udDebugFont_Deinit();

  udRender_Destroy(&s_pCurrentInstance->data.pRenderEngine);

//  udInput_Deinit();
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udViewer_RunMainLoop()
{
  udViewerDriver_RunMainLoop(s_pCurrentInstance);
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udViewer_Quit()
{
  udViewerDriver_Quit(s_pCurrentInstance);
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udViewer_SetMatrix(udRenderMatrixType type, const udFloat4x4& proj)
{
  udRender_SetMatrixF32(s_pCurrentInstance->data.pRenderView, type, proj.a);
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udViewer_SetRenderModels(udRenderModel models[], size_t numModels)
{
  for (size_t i = 0; i < numModels; ++i)
    s_pCurrentInstance->renderModels[i] = models[i];
  s_pCurrentInstance->numRenderModels = numModels;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udViewer_SetRenderOptions(const udRenderOptions &options)
{
  s_pCurrentInstance->options = options;
}

const udViewerInstanceData* udViewer_GetInstanceData()
{
  return &s_pCurrentInstance->data;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udViewer_RegisterMessageHandler(const char *pTargetName, udViewer_MessageCallback *pCallback, void *pUserData)
{
  if (s_pMessageHandlers == nullptr)
  {
    s_numMessageHandlersAllocated = 8;
    s_pMessageHandlers = udAllocType(MessageHandler, s_numMessageHandlersAllocated, udAF_None);
  }
  else if (s_numMessageHandlers == s_numMessageHandlersAllocated)
  {
    s_numMessageHandlersAllocated *= 2;
    s_pMessageHandlers = udReallocType(s_pMessageHandlers, MessageHandler, s_numMessageHandlersAllocated);
  }

  MessageHandler &m = s_pMessageHandlers[s_numMessageHandlers++];
  udStrcpy(m.name, sizeof(m.name), pTargetName);
  m.pCallback = pCallback;
  m.pUserData = pUserData;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udViewer_PostMessage(const char *pTarget, const char *pMessageType, const char *pFormat, ...)
{
  // find message handler
  MessageHandler *pHandler = nullptr;
  for (size_t i=0; i<s_numMessageHandlers; ++i)
  {
    if (!udStrcmp(pTarget, s_pMessageHandlers[i].name))
    {
      pHandler = &s_pMessageHandlers[i];
      break;
    }
  }
  if (!pHandler)
    return;

  // construct message
# define LOG_BUFFER_LEN 4096
  char buffer[LOG_BUFFER_LEN];

  va_list args;
  va_start(args, pFormat);

  size_t prefixLen = 0;
  size_t size = sizeof(buffer);
  if (pMessageType)
  {
    udStrcpy(buffer, sizeof(buffer), pMessageType);
    prefixLen = strlen(pMessageType) + 1;
    buffer[prefixLen-1] = ':';
    size -= prefixLen;
  }

#if UDPLATFORM_NACL
  vasnprintf(buffer+prefixLen, &size, pFormat, args);
#else
  vsnprintf_s(buffer+prefixLen, size, size, pFormat, args);
#endif
  buffer[LOG_BUFFER_LEN - 1] = 0;
  va_end(args);

  pHandler->pCallback(buffer, pHandler->pUserData);
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
udViewerInstance* udViewer_GetCurrentInstance()
{
  return s_pCurrentInstance;
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
void udViewer_SetCurrentInstance(udViewerInstance* pInstance)
{
  s_pCurrentInstance = pInstance;
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
void udViewer_MainLoop()
{
  udViewer_BeginFrame();
  udViewer_Update();
  udViewer_Draw();
  udViewer_EndFrame();
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
void udViewer_BeginFrame()
{
  static uint64_t lastFrame = 0;
  uint64_t now = udPerfCounterStart();
  if (lastFrame != 0)
    s_pCurrentInstance->data.timeDelta = (double)udPerfCounterMilliseconds(lastFrame, now) / 1000.0;
  lastFrame = now;

  udInput_Update();
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
void udViewer_EndFrame()
{
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
void udViewer_Update()
{
  if (s_pCurrentInstance->initParams.pUpdateCallback)
    s_pCurrentInstance->initParams.pUpdateCallback(s_pCurrentInstance->data.pUserData);
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
void udViewer_ResizeFrame(int width, int height)
{
  if (s_pCurrentInstance->data.pRenderView)
    udRender_DestroyView(&s_pCurrentInstance->data.pRenderView);
  udRender_CreateView(&s_pCurrentInstance->data.pRenderView, s_pCurrentInstance->data.pRenderEngine, width, height);

  if (s_pCurrentInstance->data.pColorBuffer)
    udFree(s_pCurrentInstance->data.pColorBuffer);
  if (s_pCurrentInstance->data.pDepthBuffer)
    udFree(s_pCurrentInstance->data.pDepthBuffer);
  size_t pitch = width * sizeof(uint32_t);
  s_pCurrentInstance->data.pColorBuffer = udAlloc(pitch * height);
  s_pCurrentInstance->data.pDepthBuffer = udAlloc(pitch * height);
  udRender_SetTarget(s_pCurrentInstance->data.pRenderView, udRTT_Color32, s_pCurrentInstance->data.pColorBuffer, (uint32_t)pitch, 0x80202080);
  udRender_SetTarget(s_pCurrentInstance->data.pRenderView, udRTT_Depth32, s_pCurrentInstance->data.pDepthBuffer, (uint32_t)pitch, 0x3f800000);

  s_pCurrentInstance->data.displayWidth = width;
  s_pCurrentInstance->data.displayHeight = height;

#if 0
  // TODO: we can calculate a render width/height here if we want to render lower-res than the display
#else
  s_pCurrentInstance->data.renderWidth = s_pCurrentInstance->data.displayWidth;
  s_pCurrentInstance->data.renderHeight = s_pCurrentInstance->data.displayHeight;
#endif

  if (s_pCurrentInstance->pColorTexture)
    udTexture_DestroyTexture(&s_pCurrentInstance->pColorTexture);
  s_pCurrentInstance->pColorTexture = udTexture_CreateTexture(udTT_2D, width, height, 1, udIF_BGRA8);

  if (s_pCurrentInstance->pDepthTexture)
    udTexture_DestroyTexture(&s_pCurrentInstance->pDepthTexture);
  s_pCurrentInstance->pDepthTexture = udTexture_CreateTexture(udTT_2D, width, height, 1, udIF_R_F32);

  if (s_pCurrentInstance->initParams.pResizeCallback)
    s_pCurrentInstance->initParams.pResizeCallback(width, height, s_pCurrentInstance->data.pUserData);
}

/*
// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
static uint32_t RenderThread(void *pThis)
{
  udRenderView *pView = nullptr;

  udDebugPrintf("Begin render thread");
  while(1)
  {
    // wait for render signal
    udWaitSemaphore(gpRenderSemaphore, -1);

    Frame &renderFrame = _this->RenderFrame();

    if(renderFrame.bTerminate)
      break;

    if(renderFrame.bResizeBuffers)
    {
      // resize render buffers
      udFree(renderFrame.colour);
      udFree(renderFrame.depth);
      renderFrame.colour = udAllocType(uint32_t, renderFrame.input.renderWidth * renderFrame.input.renderHeight, udAF_None);
      renderFrame.depth = udAllocType(float, renderFrame.input.renderWidth * renderFrame.input.renderHeight, udAF_None);
    }
    if(renderFrame.bViewChange)
    {
      if(pView)
        udRender_DestroyView(&pView);
      udRender_CreateView(&pView, _this->pRenderEngine, renderFrame.input.renderWidth, renderFrame.input.renderHeight);

      renderFrame.proj = udProjectionMatrix(renderFrame.input.fov, renderFrame.input.aspectRatio, renderFrame.input.near, renderFrame.input.far);
      udRender_SetMatrixF32(pView, udRMT_Projection, (float*)&renderFrame.proj);

      udFloat4x4 vpTransform;
      vpTransform.c[0] = udFloat4::create(renderFrame.input.renderWidth * 0.5f, 0.f, 0.f, 0.f);
      // TODO: fix the reverse projection (below) when we fix this viewport matrix!
      vpTransform.c[1] = udFloat4::create(0.f, renderFrame.input.renderHeight * 0.5f, 0.f, 0.f); // Setting this to 0.5 is incorrect, but retains previous functionality until after Spar demo. -0.5 is correct value.
      vpTransform.c[2] = udFloat4::create(0.f, 0.f, 1.f, 0.f);
      vpTransform.c[3] = udFloat4::create(renderFrame.input.renderWidth * 0.5f, renderFrame.input.renderHeight * 0.5f, 0.0f, 1.f);
      udRender_SetMatrixF32(pView, udRMT_Viewport, (float*)&vpTransform);
    }

    // setup render
    udRender_SetTarget(pView, udRTT_Color32, renderFrame.colour, renderFrame.input.renderWidth*sizeof(uint32_t));//, 0xff000080);
    udRender_SetTarget(pView, udRTT_Depth32, renderFrame.depth, renderFrame.input.renderWidth*sizeof(float), 0x3F800000);

    udRender_SetMatrixF32(pView, udRMT_Camera, (float*)&renderFrame.input.camera);

    udRenderOptions options = { sizeof(udRenderOptions), udRF_PointCubes | udRF_ClearTargets };

    // setup pick
    udRenderPick pick = { sizeof(udRenderPick) };
    uint32_t screenX = (uint32_t)((float)renderFrame.input.renderWidth  * renderFrame.input.pickX);
    uint32_t screenY = (uint32_t)((float)renderFrame.input.renderHeight * renderFrame.input.pickY);
    pick.x = screenX;
    pick.y = screenY;
    pick.highlightColor = 0;
    pick.highlightModel = nullptr;
    pick.highlightIndex = 0;
    options.pick = &pick;

    // render the scene
    udRenderModel *pRenderModels[MAX_MODELS];
    for (int i = 0; i < _this->modelCount; ++i)
      pRenderModels[i] = &_this->models[i];
#if LOG_FRAME_RATE
    uint64_t start = udPerfCounterStart();
    udRender_Render(pView, pRenderModels, modelCount, &options);
    lastFrameRenderTimeMs = udPerfCounterMilliseconds(start);
#else
    udRender_Render(pView, pRenderModels, _this->modelCount, &options);
#endif

    renderFrame.bPointFound = !!pick.found;
    if (pick.found)
    {
      // Transform the local-space point by the local matrix to bring it into world space, which we assume is meters.
      udDouble4 transPos;
      transPos.x = pick.nodePosMS[0] + pick.nodeSizeMS * 0.5f;
      transPos.y = pick.nodePosMS[1] + pick.nodeSizeMS * 0.5f;
      transPos.z = pick.nodePosMS[2] + pick.nodeSizeMS * 0.5f;
      transPos.w = 1.f;
      transPos = static_cast<const Model*>(pick.model)->local * transPos;
      pick.nodePosMS[0] = transPos.x;
      pick.nodePosMS[1] = transPos.y;
      pick.nodePosMS[2] = transPos.z;

      renderFrame.cursorWorldPos[0] = pick.nodePosMS[0];
      renderFrame.cursorWorldPos[1] = pick.nodePosMS[1];
      renderFrame.cursorWorldPos[2] = pick.nodePosMS[2];

      renderFrame.nodeSizeMS = pick.nodeSizeMS;
      renderFrame.renderModelID = pick.renderModelID;

      // reverse project the z-buffer point
      float x = renderFrame.input.pickX;
      float y = renderFrame.input.pickY;
      float z = renderFrame.depth[screenY*renderFrame.input.renderWidth + screenX];
      udFloat4 clip = { x*2.f - 1.f, y*2.f - 1.f, z, 1.f }; // TODO: fix the Y axis when we fix the viewport matrix above
      udFloat4x4 invProj = renderFrame.input.camera*udInverse(renderFrame.proj);
      udFloat4 world = invProj*clip;
      renderFrame.cursorVisualPos[0] = world.x / world.w;
      renderFrame.cursorVisualPos[1] = world.y / world.w;
      renderFrame.cursorVisualPos[2] = world.z / world.w;
    }

    udIncrementSemaphore(gpPresentSemaphore);
  }

  udDebugPrintf("Terminate render thread");
  udIncrementSemaphore(gpPresentSemaphore);
  return 0;
}
*/

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
void udViewer_Draw()
{
  if (s_pCurrentInstance->numRenderModels)
  {
    udRenderModel *pRenderModels[16];
    for(int i=0; i<s_pCurrentInstance->numRenderModels; ++i)
      pRenderModels[i] = &s_pCurrentInstance->renderModels[i];

    udRender_Render(s_pCurrentInstance->data.pRenderView, pRenderModels, (int)s_pCurrentInstance->numRenderModels, &s_pCurrentInstance->options);

    // render ud scene
    udTexture_SetImageData(s_pCurrentInstance->pColorTexture, -1, 0, s_pCurrentInstance->data.pColorBuffer);
    udTexture_SetImageData(s_pCurrentInstance->pDepthTexture, -1, 0, s_pCurrentInstance->data.pDepthBuffer);

    udShader_SetCurrent(s_shader);

    int u_texture = udShader_FindShaderParameter(s_shader, "u_texture");
    udShader_SetProgramData(0, u_texture, s_pCurrentInstance->pColorTexture);
    int u_zbuffer = udShader_FindShaderParameter(s_shader, "u_zbuffer");
    udShader_SetProgramData(1, u_zbuffer, s_pCurrentInstance->pDepthTexture);

    int u_rect = udShader_FindShaderParameter(s_shader, "u_rect");
    udShader_SetProgramData(u_rect, udFloat4::create(-1, 1, 2, -2));
    int u_textureScale = udShader_FindShaderParameter(s_shader, "u_textureScale");
    udShader_SetProgramData(u_textureScale, udFloat4::create(0, 0, 1, 1));

    udGPU_RenderVertices(s_shader, s_pQuadVB, udPT_TriangleFan, 4);

    udShader_SetCurrent(nullptr);
  }

  if (s_pCurrentInstance->initParams.pRenderCallback)
    s_pCurrentInstance->initParams.pRenderCallback(&s_pCurrentInstance->options, s_pCurrentInstance->renderModels, s_pCurrentInstance->numRenderModels, s_pCurrentInstance->data.pUserData);
}

#endif
