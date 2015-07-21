
#include "udView.h"
#include "udKernel.h"
#include "udScene.h"
#include "udCamera.h"

#include "udRenderScene.h"

static const udPropertyDesc props[] =
{
  {
    "camera", // id
    "Camera", // displayName
    "Camera for viewport", // description
    nullptr, // getter
    nullptr, // setter
    udTypeDesc(udPropertyType::Component) // type
  }
};
const udComponentDesc udView::descriptor =
{
  &udComponent::descriptor, // pSuperDesc

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "view",      // id
  "udView",    // displayName
  "Is a view", // description

  [](){ return udR_Success; },  // pInit
  udView::Create,               // pCreateInstance

  udSlice<const udPropertyDesc>(props, UDARRAYSIZE(props)) // propeties
};

udResult udView::InputEvent(const udInputEvent &ev)
{
  // if view wants to handle anything personally
  //...
  if (spScene)
  {
      // pass to scene
    udResult r = spScene->InputEvent(ev);
    if (r == udR_EventNotHandled)
    {
      // pass to camera
      r = spCamera->InputEvent(ev);
    }

    return r;
  }

  return udR_EventNotHandled;
}

udResult udView::Resize(int width, int height)
{
  int oldRenderWidth = renderWidth;
  int oldRenderHeight = renderHeight;

  // calculate sizes
  displayWidth = width;
  displayHeight = height;
#if 0
  // TODO: we can calculate a render width/height here if we want to render lower-res than the display
#else
  renderWidth = displayWidth;
  renderHeight = displayHeight;
#endif

  // dirty the viewport if the resize affected the render buffers
  if (oldRenderWidth != renderWidth || oldRenderHeight != renderHeight)
    bDirty = true;

  if (pResizeCallback)
    pResizeCallback(udViewRef(this), width, height);

  return udR_Success;
}

udResult udView::Render()
{
  if (pPreRenderCallback)
    pPreRenderCallback(udViewRef(this), spScene);

  udResult r = udR_Success;

  GetRenderableView()->RenderGPU();

  if (pPostRenderCallback)
    pPostRenderCallback(udViewRef(this), spScene);

  return r;
}

udSharedPtr<const udRenderableView> udView::GetRenderableView()
{
  if (!bDirty)
    return spCache;

  udRenderableView *pView = new udRenderableView;

  pView->spView = udViewRef(this);

  pView->view = spCamera->GetViewMatrix();
  spCamera->GetProjectionMatrix((double)displayWidth / (double)displayHeight, &pView->projection);

  pView->scene = spScene->GetRenderScene();
  pView->options = options;

  pView->pRenderEngine = pKernel->GetRenderEngine();

  pView->displayWidth = displayWidth;
  pView->displayHeight = displayHeight;
  pView->renderWidth = renderWidth;
  pView->renderHeight = renderHeight;

  // TODO: move UD to async render on another thread
  pView->RenderUD();

  spCache = udSharedPtr<const udRenderableView>(pView);
  bDirty = false;
  return spCache;
}

void udView::SetScene(udSceneRef spNewScene)
{
  if (spScene == spNewScene)
    return;

  if (spScene)
    spScene->Dirty.Unsubscribe(this, &udView::OnDirty);

  spScene = spNewScene;

  if (spScene)
    spScene->Dirty.Subscribe(this, &udView::OnDirty);
}

void udView::SetCamera(udCameraRef spCamera)
{
  this->spCamera = spCamera;
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

void udView::OnDirty()
{
  bDirty = true;
  Dirty.Signal();
}
