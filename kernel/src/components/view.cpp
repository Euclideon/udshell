#include "view.h"
#include "kernel.h"
#include "scene.h"
#include "nodes/camera.h"
#include "renderscene.h"

namespace ep {

Array<const PropertyInfo> View::GetProperties()
{
  return{
    EP_MAKE_PROPERTY(Camera, "Camera for viewport", nullptr, 0),
    EP_MAKE_PROPERTY(Scene, "Scene for viewport", nullptr, 0),
    EP_MAKE_PROPERTY(EnablePicking, "Enable Picking", nullptr, 0),
    EP_MAKE_PROPERTY_RO(MousePosition, "Mouse Position", nullptr, 0)
  };
}

bool View::InputEvent(const epInputEvent &ev)
{
  bool handled = false;
  if (ev.deviceType == epID_Mouse)
  {
    if (ev.eventType == epInputEvent::Move)
    {
      mousePosition.x = (uint32_t)ev.move.xAbsolute;
      mousePosition.y = (uint32_t)ev.move.yAbsolute;
      MousePositionChanged.Signal(mousePosition);
    }
  }
  else if (ev.deviceType == epID_Keyboard)
  {
    if  (ev.key.key == epKC_P && ev.key.state == 0)
      SetEnablePicking(!pickingEnabled);
  }

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
  if (spLatestFrame->pickingEnabled)
  {
    udRenderPick &pick = spLatestFrame->udPick;
    if (pick.found)
    {
      Double4 transPos = {
                           pick.nodePosMS[0] + pick.childSizeMS,
                           pick.nodePosMS[1] + pick.childSizeMS,
                           pick.nodePosMS[2] + pick.childSizeMS,
                           1.0
                         };

      pickedPoint = (static_cast<const UDRenderState*>(pick.model)->matrix * transPos).toVector3();
      pickHighlightData = { pick.model, pick.nodeIndex };
      PickFound.Signal(pickedPoint);
    }
    else
    {
      pickedPoint = { 0 , 0, 0 };
      pickHighlightData = { nullptr, 0, };
    }
  }
  FrameReady.Signal();
}

void View::GoToBookmark(String bookmarkName)
{
  if (spScene && spCamera)
  {
    const Bookmark *pBM = spScene->FindBookMark(bookmarkName);
    if (pBM)
    {
      spCamera->SetMatrix(Double4x4::rotationYPR(pBM->ypr, pBM->position));
      ForceDirty();
    }
  }
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

    Renderer *pRenderer = ((kernel::Kernel&)GetKernel()).GetRenderer();
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

    if (pickingEnabled)
    {
      spRenderView->pickingEnabled = true;
      spRenderView->udPick.x = mousePosition.x;
      spRenderView->udPick.y = mousePosition.y;
      spRenderView->udPick.highlightIndex = pickHighlightData.highlightIndex;
      spRenderView->udPick.highlightModel = pickHighlightData.highlightModel;
      spRenderView->udPick.highlightColor = 0xFFC0C000;
    }

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
  if ((spCamera && spCamera->Update(timeStep)) || pickingEnabled)
    ForceDirty();
}

void View::Activate()
{
  GetKernel().UpdatePulse.Subscribe(Delegate<void(double)>(this, &View::Update));
}

void View::Deactivate()
{
  GetKernel().UpdatePulse.Unsubscribe(Delegate<void(double)>(this, &View::Update));
}

// TODO: Implement this
void View::RequestPick(SharedArray<const ScreenPoint> points, const PickDelegate &del)
{
#if 0 // TODO: enable this once picking with an array of points is supported
  pickRequests.pushBack() = { points, del } ;
#endif
}

} // namespace ep
