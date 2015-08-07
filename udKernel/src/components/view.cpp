
#include "view.h"
#include "kernel.h"
#include "scene.h"
#include "nodes/camera.h"
#include "renderscene.h"

namespace ud
{

static CPropertyDesc props[] =
{
  {
    {
      "camera", // id
      "Camera", // displayName
      "Camera for viewport", // description
    },
    nullptr, // getter
    nullptr, // setter
  }
};
static CEventDesc events[] =
{
  {
    {
      "dirty", // id
      "Dirty", // displayName
      "View dirty event", // description
    },
    &View::Dirty
  }
};
ComponentDesc View::descriptor =
{
  &Component::descriptor, // pSuperDesc

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "view",      // id
  "View",    // displayName
  "Is a view", // description

  udSlice<CPropertyDesc>(props, UDARRAYSIZE(props)), // properties
  nullptr,
  udSlice<CEventDesc>(events, UDARRAYSIZE(events)) // events
};

udResult View::InputEvent(const udInputEvent &ev)
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

udResult View::Resize(int width, int height)
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
    pResizeCallback(ViewRef(this), width, height);

  return udR_Success;
}

udResult View::Render()
{
  if (pPreRenderCallback)
    pPreRenderCallback(ViewRef(this), spScene);

  udResult r = udR_Success;

  GetRenderableView()->RenderGPU();

  if (pPostRenderCallback)
    pPostRenderCallback(ViewRef(this), spScene);

  return r;
}

RenderableViewRef View::GetRenderableView()
{
  if (!bDirty)
    return spCache;

  spCache = RenderableViewRef::create();

  spCache->spView = ViewRef(this);

  spCache->view = spCamera->GetViewMatrix();
  spCamera->GetProjectionMatrix((double)displayWidth / (double)displayHeight, &spCache->projection);

  spCache->spScene = spScene->GetRenderScene();
  spCache->options = options;

  spCache->pRenderEngine = pKernel->GetRenderEngine();

  spCache->displayWidth = displayWidth;
  spCache->displayHeight = displayHeight;
  spCache->renderWidth = renderWidth;
  spCache->renderHeight = renderHeight;

  // TODO: move UD to async render on another thread
  spCache->RenderUD();

  bDirty = false;
  return spCache;
}

void View::SetScene(SceneRef spNewScene)
{
  if (spScene == spNewScene)
    return;

  if (spScene)
    spScene->Dirty.Unsubscribe(this, &View::OnDirty);

  spScene = spNewScene;

  if (spScene)
    spScene->Dirty.Subscribe(this, &View::OnDirty);
}

void View::SetCamera(CameraRef spCamera)
{
  this->spCamera = spCamera;
}

void View::GetDimensions(int *pWidth, int *pHeight) const
{
  if (pWidth)
    *pWidth = displayWidth;
  if (pHeight)
    *pHeight = displayHeight;
}

void View::GetRenderDimensions(int *pWidth, int *pHeight) const
{
  if (pWidth)
    *pWidth = renderWidth;
  if (pHeight)
    *pHeight = renderHeight;
}

void View::OnDirty()
{
  bDirty = true;
  Dirty.Signal();
}

} // namespace ud
