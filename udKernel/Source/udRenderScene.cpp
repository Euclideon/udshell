#include "udRenderScene.h"

#include "udGPU.h"
#include "udTexture.h"
#include "udVertex.h"
#include "udShader.h"


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

static udVertexDeclaration *s_pPosUV = nullptr;
static udVertexBuffer *s_pQuadVB = nullptr;
static udShaderProgram *s_shader = nullptr;

void udRenderScene_InitRender()
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
}


udRenderableView::~udRenderableView()
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

void udRenderableView::RenderUD()
{
  if (scene->ud.length)
  {
    size_t size = sizeof(udRenderModel)*scene->ud.length + sizeof(udRenderModel*)*scene->ud.length;
    udRenderModel **ppRenderModels = (udRenderModel**)alloca(size);
    udRenderModel *pRenderModels = (udRenderModel *)&ppRenderModels[scene->ud.length];
    memset(pRenderModels, 0, sizeof(udRenderModel)*scene->ud.length);

    // construct the awkward list of ud models
    for (size_t i = 0; i < scene->ud.length; ++i)
    {
      ppRenderModels[i] = pRenderModels + i;

      pRenderModels[i].pOctree = scene->ud[i].pOctree;
      pRenderModels[i].pWorldMatrixD = scene->ud[i].matrix.a;
    }

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
    udRender_Render(pRenderView, ppRenderModels, (int)scene->ud.length, &options);
  }
  else
  {
    uint32_t colorPitch = renderWidth*sizeof(uint32_t);
    uint32_t depthPitch = renderWidth*sizeof(float);
    pColorBuffer = udAllocFlags(colorPitch * renderHeight, udAF_Zero);
    pDepthBuffer = udAllocFlags(depthPitch * renderHeight, udAF_Zero);
  }
}

void udRenderableView::RenderGPU() const
{
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

  udGPU_RenderVertices(s_shader, s_pQuadVB, udPT_TriangleFan, 4);

  // TODO: if Qt, destroy textures immediately
  //...
  // pColorTexture = nullptr;
}
