
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

bool View::InputEvent(const udInputEvent &ev)
{
  bool handled = false;

  // if view wants to handle anything personally
  //...

  if (spScene)
    handled = spScene->InputEvent(ev);
  if (!handled && spCamera)
    handled = spCamera->ViewportInputEvent(ev);
  return handled;
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
    OnDirty();

  if (pResizeCallback)
    pResizeCallback(ViewRef(this), width, height);

  return udR_Success;
}

RenderableViewRef View::GetRenderableView()
{
  return spLatestFrame;
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

  OnDirty();
}

void View::SetCamera(CameraRef spCamera)
{
  this->spCamera = spCamera;

  OnDirty();
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

void View::SetLatestFrame(udUniquePtr<RenderableView> spFrame)
{
  spLatestFrame = spFrame;
  FrameReady.Signal();
}

void View::OnDirty()
{
  if (spScene && spCamera)
  {
    udUniquePtr<RenderableView> spRenderView = udUniquePtr<RenderableView>(new RenderableView);

    Renderer *pRenderer = pKernel->GetRenderer();
    spRenderView->pRenderEngine = pRenderer->GetRenderEngine();

    spRenderView->spView = ViewRef(this);

    spRenderView->camera = spCamera->GetCameraMatrix();
    spCamera->GetProjectionMatrix((double)displayWidth / (double)displayHeight, &spRenderView->projection);

    spRenderView->spScene = spScene->GetRenderScene();
    spRenderView->options = options;

    spRenderView->displayWidth = displayWidth;
    spRenderView->displayHeight = displayHeight;
    spRenderView->renderWidth = renderWidth;
    spRenderView->renderHeight = renderHeight;

    // if there are ud jobs, we'll need to send it to the UD render thread
    if (spRenderView->spScene->ud.length > 0)
      pRenderer->AddUDRenderJob(spRenderView);
    else
      SetLatestFrame(spRenderView);
  }

  // emit the dirty signal
  Dirty.Signal();
}

} // namespace ud
