#include "udPlatform.h"
#include "udPlatformUtil.h"
#include "udRender.h"
#include "udGPU.h"
#include "udTexture.h"
#include "udVertex.h"
#include "udShader.h"
#include "udInput.h"
#include "udViewer_Internal.h"
#include "udMath.h"

#include "udCamera.h"


void udViewerDriver_Init(int argc, char* argv[]);
void udViewerDriver_RunMainLoop();
void udViewerDriver_Quit();
void udDebugFont_InitModule();
void udDebugFont_DeinitModule();

udViewerInitParams s_initParams;

udRenderEngine *s_renderEngine;
udRenderView *s_renderView;

/*static*/ void *s_colorBuffer;
/*static*/ void *s_depthBuffer;
static int s_viewWidth, s_viewHeight;

int s_viewW, s_viewH; // <-- *** DELETE MEE ***

udVertexDeclaration *s_pPosUV;

udVertexBuffer *s_pQuadVB;
udTexture *s_pColor;
udTexture *s_pDepth;
udShaderProgram *s_shader;

double s_timeDelta;

static udSemaphore *s_pRenderSemaphore;
static udSemaphore *s_pPresentSemaphore;

static udRenderModel s_renderModels[16];
static size_t s_numRenderModels;
static udRenderOptions s_options;

RegisteredCallback s_udViewerCallbacks[udVCT_Max];

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
void udViewer_RegisterCallback(udViewerCallbackType type, udViewerCallback *pCallback, void *pUserData)
{
  s_udViewerCallbacks[type].pCallback = pCallback;
  s_udViewerCallbacks[type].pUserData = pUserData;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udViewer_Init(const udViewerInitParams &initParams)
{
  udRender_Create(&s_renderEngine, initParams.renderThreadCount);

  s_initParams = initParams;
  udViewerDriver_Init(initParams.argc, initParams.argv);

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

  udDebugFont_InitModule();
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udViewer_Deinit()
{
  udDebugFont_DeinitModule();

  udRender_Destroy(&s_renderEngine);

//  udInput_Deinit();
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udViewer_RunMainLoop()
{
  udViewerDriver_RunMainLoop();
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udViewer_Quit()
{
  udViewerDriver_Quit();
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udViewer_SetRenderModels(udRenderModel models[], size_t numModels)
{
  for(size_t i = 0; i < numModels; ++i)
    s_renderModels[i] = models[i];
  s_numRenderModels = numModels;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udViewer_SetRenderOptions(const udRenderOptions &options)
{
  s_options = options;
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
  if(lastFrame != 0)
    s_timeDelta = (double)udPerfCounterMilliseconds(lastFrame, now) / 1000.0;
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
  if(s_udViewerCallbacks[udVCT_Update].pCallback)
    s_udViewerCallbacks[udVCT_Update].pCallback(s_udViewerCallbacks[udVCT_Update].pUserData);
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
void udViewer_ResizeFrame(int width, int height)
{
  if (s_renderView)
    udRender_DestroyView(&s_renderView);
  udRender_CreateView(&s_renderView, s_renderEngine, width, height);

  if (s_colorBuffer)
    udFree(s_colorBuffer);
  if (s_depthBuffer)
    udFree(s_depthBuffer);
  size_t pitch = width * sizeof(uint32_t);
  s_colorBuffer = udAlloc(pitch * height);
  s_depthBuffer = udAlloc(pitch * height);
  udRender_SetTarget(s_renderView, udRTT_Color32, s_colorBuffer, (uint32_t)pitch, 0x80202080);
  udRender_SetTarget(s_renderView, udRTT_Depth32, s_depthBuffer, (uint32_t)pitch, 0x3f800000);

  s_viewWidth = width;
  s_viewHeight = height;
  s_viewW = width; // DELETE MEEEE!
  s_viewH = height;

  if(s_pColor)
    udTexture_DestroyTexture(&s_pColor);
  s_pColor = udTexture_CreateTexture(udTT_2D, width, height, 1, udIF_BGRA8);

  if(s_pDepth)
    udTexture_DestroyTexture(&s_pDepth);
  s_pDepth = udTexture_CreateTexture(udTT_2D, width, height, 1, udIF_R_F32);

  if(s_udViewerCallbacks[udVCT_Resize].pCallback)
  {
    udResizeCallbackData data;
    data.pUserData = s_udViewerCallbacks[udVCT_Resize].pUserData;
    data.width = width;
    data.height = height;
    s_udViewerCallbacks[udVCT_Resize].pCallback(&data);
  }
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
  udRenderModel *pRenderModels[16];
  for(int i=0; i<s_numRenderModels; ++i)
    pRenderModels[i] = &s_renderModels[i];

  udRender_Render(s_renderView, pRenderModels, (int)s_numRenderModels, &s_options);

  // render ud scene
  udTexture_SetImageData(s_pColor, -1, 0, s_colorBuffer);
  udTexture_SetImageData(s_pDepth, -1, 0, s_depthBuffer);

  udShader_SetCurrent(s_shader);

  int u_texture = udShader_FindShaderParameter(s_shader, "u_texture");
  udShader_SetProgramData(0, u_texture, s_pColor);
  int u_zbuffer = udShader_FindShaderParameter(s_shader, "u_zbuffer");
  udShader_SetProgramData(1, u_zbuffer, s_pDepth);

  int u_rect = udShader_FindShaderParameter(s_shader, "u_rect");
  udShader_SetProgramData(u_rect, udFloat4::create(-1, 1, 2, -2));
  int u_textureScale = udShader_FindShaderParameter(s_shader, "u_textureScale");
  udShader_SetProgramData(u_textureScale, udFloat4::create(0, 0, 1, 1));

  udGPU_RenderVertices(s_shader, s_pQuadVB, udPT_TriangleFan, 4);

  udShader_SetCurrent(NULL);

  if(s_udViewerCallbacks[udVCT_Draw].pCallback)
  {
    udRenderCallbackData renderData;
    renderData.pUserData = s_udViewerCallbacks[udVCT_Draw].pUserData;
    renderData.pRenderOptions = &s_options;
    renderData.pRenderModels = s_renderModels;
    renderData.numRenderModels = s_numRenderModels;
    s_udViewerCallbacks[udVCT_Draw].pCallback(&renderData);
  }
}
