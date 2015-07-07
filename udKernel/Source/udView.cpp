
#include "udView.h"
#include "udKernel.h"
#include "udScene.h"
#include "udCamera.h"

#include "udGPU.h"
#include "udTexture.h"
#include "udVertex.h"
#include "udShader.h"


udVertexDeclaration *s_pPosUV;
udVertexBuffer *s_pQuadVB;
udShaderProgram *s_shader;


static const udPropertyDesc props[] =
{
  {
    "camera", // id
    "Camera", // displayName
    "Camera for viewport", // description
    udPropertyType::Integer, // type
    0, // arrayLength
    udPF_NoRead, // flags
    udPropertyDisplayType::Default // displayType
  }
};
const udComponentDesc udView::descriptor =
{
  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  udComponentType::View, // type

  "view",      // id
  "component", // parentId
  "udView",    // displayName
  "Is a view", // description

  [](){ return udR_Success; },  // pInit
  udView::InitRender,           // pInitRender
  udView::Create,               // pCreateInstance

  props,                           // pProperties
  sizeof(props) / sizeof(props[0]) // numProperties
};

udView::~udView()
{
  if (pRenderView)
    udRender_DestroyView(&pRenderView);
  if (pColorBuffer)
    udFree(pColorBuffer);
  if (pDepthBuffer)
    udFree(pDepthBuffer);
}

udResult udView::InputEvent(const udInputEvent &ev)
{
  // if view wants to handle anything personally
  //...
  if (pScene)
  {
      // pass to scene
    udResult r = pScene->InputEvent(ev);
    if (r == udR_EventNotHandled)
    {
      // pass to camera
      r = pCamera->InputEvent(ev);
    }

    return r;
  }

  return udR_EventNotHandled;
}

udResult udView::Resize(int width, int height)
{
  // clean up
  if (pRenderView)
    udRender_DestroyView(&pRenderView);
  if (pColorBuffer)
    udFree(pColorBuffer);
  if (pDepthBuffer)
    udFree(pDepthBuffer);

  // calculate sized
  displayWidth = width;
  displayHeight = height;
#if 0
  // TODO: we can calculate a render width/height here if we want to render lower-res than the display
#else
  renderWidth = displayWidth;
  renderHeight = displayHeight;
#endif

  if (pKernel->GetRenderEngine()) // HACK: REMOVE THIS IF BLOCK. is hack for udWebView
  {
    // allocate new stuff
    udRender_CreateView(&pRenderView, pKernel->GetRenderEngine(), renderWidth, renderHeight);

    size_t pitch = renderWidth * sizeof(uint32_t);
    pColorBuffer = udAlloc(pitch * renderHeight);
    pDepthBuffer = udAlloc(pitch * renderHeight);
    udRender_SetTarget(pRenderView, udRTT_Color32, pColorBuffer, (uint32_t)pitch, 0x80202080);
    udRender_SetTarget(pRenderView, udRTT_Depth32, pDepthBuffer, (uint32_t)pitch, 0x3f800000);

    if (pColorTexture)
      udTexture_DestroyTexture(&pColorTexture);
    pColorTexture = udTexture_CreateTexture(udTT_2D, renderWidth, renderHeight, 1, udIF_BGRA8);

    if (pDepthTexture)
      udTexture_DestroyTexture(&pDepthTexture);
    pDepthTexture = udTexture_CreateTexture(udTT_2D, renderWidth, renderHeight, 1, udIF_R_F32);
  }

  if (pResizeCallback)
    pResizeCallback(this, width, height);

  return udR_Success;
}

udResult udView::Render()
{
  if (!pRenderView)
  {
    // resize(w, h);
  }

  if (pPreRenderCallback)
    pPreRenderCallback(this, pScene.ptr());

  udResult r = udR_Success;

  if (pScene)
  {
    // prepare the render view
    udDouble4x4 mat;
    pCamera->GetProjectionMatrix(GetAspectRatio(), &mat);
    udRender_SetMatrixF64(pRenderView, udRMT_Projection, mat.a);

    pCamera->GetCameraMatrix(&mat);
    udRender_SetMatrixF64(pRenderView, udRMT_Camera, mat.a);

    udRender_SetTarget(pRenderView, udRTT_Color32, pColorBuffer, renderWidth*sizeof(uint32_t));//, 0xff000080);
    udRender_SetTarget(pRenderView, udRTT_Depth32, pDepthBuffer, renderWidth*sizeof(float), 0x3F800000);

    // render the scene
    r = pScene->Render(this);

    if (r == udR_Success)
    {
      // blit the scene to the viewport
      udTexture_SetImageData(pColorTexture, -1, 0, pColorBuffer);
      udTexture_SetImageData(pDepthTexture, -1, 0, pDepthBuffer);

      udShader_SetCurrent(s_shader);

      int u_texture = udShader_FindShaderParameter(s_shader, "u_texture");
      udShader_SetProgramData(0, u_texture, pColorTexture);
      int u_zbuffer = udShader_FindShaderParameter(s_shader, "u_zbuffer");
      udShader_SetProgramData(1, u_zbuffer, pDepthTexture);

      int u_rect = udShader_FindShaderParameter(s_shader, "u_rect");
      udShader_SetProgramData(u_rect, udFloat4::create(-1, 1, 2, -2));
      int u_textureScale = udShader_FindShaderParameter(s_shader, "u_textureScale");
      udShader_SetProgramData(u_textureScale, udFloat4::create(0, 0, 1, 1));

      udGPU_RenderVertices(s_shader, s_pQuadVB, udPT_TriangleFan, 4);
    }
    else
    {
      // TODO: render something to indicate a failed render...
    }
  }

  if (pPostRenderCallback)
    pPostRenderCallback(this, pScene.ptr());

  return r;
}

void udView::GetDimensions(int *pWidth, int *pHeight) const
{
  if (pWidth)
    *pWidth = displayWidth;
  if (pHeight)
    *pHeight = displayHeight;
}

void udView::GetRenderDimensions(int *pWidth, int *pHeight) const
{
  if (pWidth)
    *pWidth = renderWidth;
  if (pHeight)
    *pHeight = renderHeight;
}


// init the renderer stuff

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

udResult udView::InitRender()
{
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
      { 0, 0 },
      { 1, 0 },
      { 1, 1 },
      { 0, 1 }
  };
  udVertex_SetVertexBufferData(s_pQuadVB, quad, sizeof(quad));

  udShader *pVS = udShader_CreateShader(s_vertexShader, sizeof(s_vertexShader), udST_VertexShader);
  udShader *pPS = udShader_CreateShader(s_blitShader, sizeof(s_blitShader), udST_PixelShader);
  s_shader = udShader_CreateShaderProgram(pVS, pPS);

  return udR_Success;
}
