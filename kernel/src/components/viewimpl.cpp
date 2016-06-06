#include "viewimpl.h"
#include "kernelimpl.h"

namespace ep {

Array<const PropertyInfo> View::GetProperties() const
{
  return{
    EP_MAKE_PROPERTY(Camera, "Camera for viewport", nullptr, 0),
    EP_MAKE_PROPERTY(Scene, "Scene for viewport", nullptr, 0),
    EP_MAKE_PROPERTY(EnablePicking, "Enable Picking", nullptr, 0),
    EP_MAKE_PROPERTY_RO(MousePosition, "Mouse Position", nullptr, 0),
    EP_MAKE_PROPERTY_RO(AspectRatio, "Aspect ratio", nullptr, 0),
    EP_MAKE_PROPERTY_RO(Dimensions, "The height and width of the View", nullptr, 0),
    EP_MAKE_PROPERTY_RO(RenderDimensions, "The resolution of the rendered content", nullptr, 0),
  };
}

Array<const MethodInfo> View::GetMethods() const
{
  return{
    EP_MAKE_METHOD(GoToBookmark, "Move the Camera to the specified Bookmark"),
    EP_MAKE_METHOD(Activate, "Activate the View, e.g. start rendering"),
    EP_MAKE_METHOD(Deactivate, "Deactivate the View, e.g. stop rendering"),
    EP_MAKE_METHOD(Resize, "Resize the View"),
  };
}

Array<const EventInfo> View::GetEvents() const
{
  return{
    EP_MAKE_EVENT(Dirty, "View dirty event"),
    EP_MAKE_EVENT(FrameReady, "The next frame has finished rendering"),
    EP_MAKE_EVENT(EnabledPickingChanged, "Enable Picking changed"),
    EP_MAKE_EVENT(PickFound, "Pick found"),
    EP_MAKE_EVENT(MousePositionChanged, "Mouse Position changed")
  };
}

void ViewImpl::SetScene(SceneRef spNewScene)
{
  if (spScene == spNewScene)
    return;

  if (spScene)
    spScene->Dirty.Unsubscribe(this, &ViewImpl::OnDirty);

  spScene = spNewScene;

  if (spScene)
    spScene->Dirty.Subscribe(this, &ViewImpl::OnDirty);

  OnDirty();
}

void ViewImpl::SetEnablePicking(bool enable)
{
  if (pickingEnabled != enable)
  {
    pickingEnabled = enable;
    pInstance->EnabledPickingChanged.Signal(pickingEnabled);
  }
}

void ViewImpl::Resize(int width, int height)
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
}

void ViewImpl::GoToBookmark(String bookmarkName)
{
  if (spScene && spCamera)
  {
    const Bookmark *pBM = spScene->FindBookmark(bookmarkName);
    if (pBM)
    {
      spCamera->SetMatrix(Double4x4::rotationYPR(pBM->ypr, pBM->position));
      OnDirty();
    }
  }
}

bool ViewImpl::InputEvent(const ep::InputEvent &ev)
{
  if (ev.deviceType == InputDevice::Mouse)
  {
    if (ev.eventType == ep::InputEvent::EventType::Move)
    {
      mousePosition.x = (uint32_t)ev.move.xAbsolute;
      mousePosition.y = (uint32_t)ev.move.yAbsolute;
      pInstance->MousePositionChanged.Signal(mousePosition);
    }
  }
  else if (ev.deviceType == ep::InputDevice::Keyboard)
  {
    if ((KeyCode)ev.key.key == KeyCode::P && ev.key.state == 0)
      SetEnablePicking(!pickingEnabled);
  }

  bool handled = inputEventHook ? inputEventHook(ev) : false;

  if (!handled && spScene)
    handled = spScene->InputEvent(ev);
  if (!handled && spCamera)
    handled = spCamera->ViewportInputEvent(ev);
  return handled;
}

void ViewImpl::OnDirty()
{
  if (renderWidth == 0 || renderHeight == 0)
  {
    GetKernel()->LogWarning(1, "Render target has zero size! Unable to render...");
    return;
  }

  if (spScene && spCamera)
  {
    SharedPtr<Renderer> spRenderer = GetKernel()->GetImpl()->GetRenderer();

    UniquePtr<RenderableView> spRenderView = UniquePtr<RenderableView>::create(spRenderer);

    spRenderView->pRenderEngine = spRenderer->GetRenderEngine();

    spRenderView->spView = ViewRef(pInstance);

    spRenderView->camera = spCamera->GetCameraMatrix();
    spCamera->GetProjectionMatrix((double)displayWidth / (double)displayHeight, &spRenderView->projection);

    spRenderView->spScene = spScene->GetRenderScene();

    spRenderView->options = udRenderOptions{ sizeof(udRenderOptions), GetRenderableUDFlags(), nullptr, nullptr,
                                             nullptr, nullptr, nullptr, nullptr };

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
    {
      spRenderView->spColorBuffer = spRenderer->AllocRenderBuffer();
      spRenderView->spColorBuffer->Allocate<uint32_t>(Slice<const size_t>{ (size_t)renderWidth, (size_t)renderHeight });
      spRenderView->spDepthBuffer = spRenderer->AllocRenderBuffer();
      spRenderView->spDepthBuffer->Allocate<float>(Slice<const size_t>{ (size_t)renderWidth, (size_t)renderHeight });
      spRenderer->AddUDRenderJob(std::move(spRenderView));
    }
    else
      SetLatestFrame(std::move(spRenderView));
  }

  // emit the dirty signal
  pInstance->Dirty.Signal();
}

void ViewImpl::SetLatestFrame(UniquePtr<RenderableView> spFrame)
{
  spLatestFrame = spFrame;
  if (spLatestFrame && spLatestFrame->pickingEnabled)
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

      pickedPoint = (reinterpret_cast<const UDRenderContext*>(pick.model)->matrix * transPos).toVector3();

      pickHighlightData = { pick.model, pick.nodeIndex };
      pInstance->PickFound.Signal(pickedPoint);
    }
    else
    {
      pickedPoint = { 0 , 0, 0 };
      pickHighlightData = { nullptr, 0, };
    }
  }
  pInstance->FrameReady.Signal();
}

} // namespace ep
