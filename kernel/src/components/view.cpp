#include "view.h"
#include "kernel.h"
#include "scene.h"
#include "nodes/camera.h"
#include "renderscene.h"

namespace kernel {

Array<const PropertyInfo> View::GetProperties()
{
  return{
    EP_MAKE_PROPERTY(Camera, "Camera for viewport", nullptr, 0),
  };
}

bool View::InputEvent(const epInputEvent &ev)
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

epResult View::Resize(int width, int height)
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

  return epR_Success;
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

void View::SetCamera(CameraRef _spCamera)
{
  spCamera = _spCamera;

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

void View::SetLatestFrame(UniquePtr<RenderableView> spFrame)
{
  spLatestFrame = spFrame;
  FrameReady.Signal();
}

void View::OnDirty()
{
  if (renderWidth == 0 || renderHeight == 0)
  {
    pKernel->LogWarning(1, "Render target has zero size! Unable to render...");
    return;
  }

  if (spScene && spCamera)
  {
    UniquePtr<RenderableView> spRenderView = UniquePtr<RenderableView>(new RenderableView);

    Renderer *pRenderer = GetKernel().GetRenderer();
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

void View::Update(double timeStep)
{
  if (spCamera)
  {
    if (spCamera->Update(timeStep))
      ForceDirty();
  }
}

void View::Activate()
{
  GetKernel().UpdatePulse.Subscribe(updateFunc);
}

void View::Deactivate()
{
  GetKernel().UpdatePulse.Unsubscribe(updateFunc);
}

} // namespace kernel
