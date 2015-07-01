
#include "udView.h"
#include "udKernel.h"
#include "udScene.h"
#include "udCamera.h"

#include "udGPU.h"
#include "udTexture.h"


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

  [](){ return udR_Success; },             // pInit
  [](){ return udR_Success; },             // pInitRender
  udView::Create, // pCreateInstance

  props,                           // pProperties
  sizeof(props) / sizeof(props[0]) // numProperties
};


udResult udView::InputEvent(const udInputEvent &ev)
{
  // pass to scene
  pScene->InputEvent(ev);

  // pass to camera

  return udR_Success;
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

  udResult r = udR_Success;

  // prepare the render view
  udDouble4x4 mat;
  pCamera->GetProjectionMatrix(GetAspectRatio(), &mat);
  udRender_SetMatrixF64(pRenderView, udRMT_Projection, mat.a);

  pCamera->GetCameraMatrix(&mat);
  udRender_SetMatrixF64(pRenderView, udRMT_Camera, mat.a);

  udRender_SetTarget(pRenderView, udRTT_Color32, pColorBuffer, renderWidth*sizeof(uint32_t));//, 0xff000080);
  udRender_SetTarget(pRenderView, udRTT_Depth32, pDepthBuffer, renderWidth*sizeof(float), 0x3F800000);

  if (pScene)
  {
    // render the scene
    r = pScene->Render(this);

    if (udR_Success)
    {
      // blit the scene to the viewport
      udTexture_SetImageData(pColorTexture, -1, 0, pColorBuffer);
      udTexture_SetImageData(pDepthTexture, -1, 0, pDepthBuffer);
/*
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
*/
    }
    else
    {
      // TODO: render something to indicate a failed render...
    }
  }

  if (pRenderCallback)
    pRenderCallback(this, pScene);

  return r;
}

void udView::GetDimensions(int *pWidth, int *pHeight) const
{
  if (pWidth)
    *pWidth = displayWidth;
  if (pHeight)
    *pHeight = displayHeight;
}
