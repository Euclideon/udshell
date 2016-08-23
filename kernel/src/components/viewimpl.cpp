#include "viewimpl.h"
#include "kernelimpl.h"
#include "ep/cpp/component/node/node.h"

namespace ep {

Array<const PropertyInfo> View::getProperties() const
{
  return{
    EP_MAKE_PROPERTY("camera", getCamera, setCamera, "Camera for viewport", nullptr, 0),
    EP_MAKE_PROPERTY("scene", getScene, setScene, "Scene for viewport", nullptr, 0),
    EP_MAKE_PROPERTY("enablePicking", getEnablePicking, setEnablePicking, "Enable Picking", nullptr, 0),
    EP_MAKE_PROPERTY_RO("mousePosition", getMousePosition, "Mouse Position", nullptr, 0),
    EP_MAKE_PROPERTY_RO("aspectRatio", getAspectRatio, "Aspect ratio", nullptr, 0),
    EP_MAKE_PROPERTY("clearColor", getClearColor, setClearColor, "Clear Color", nullptr, 0),
    EP_MAKE_PROPERTY_RO("dimensions", getDimensions, "The height and width of the View", nullptr, 0),
    EP_MAKE_PROPERTY_RO("renderDimensions", getRenderDimensions, "The resolution of the rendered content", nullptr, 0),
  };
}

Array<const MethodInfo> View::getMethods() const
{
  return{
    EP_MAKE_METHOD(gotoBookmark, "Move the Camera to the specified Bookmark"),
    EP_MAKE_METHOD(activate, "Activate the View, e.g. start rendering"),
    EP_MAKE_METHOD(deactivate, "Deactivate the View, e.g. stop rendering"),
    EP_MAKE_METHOD(resize, "Resize the View"),
  };
}

Array<const EventInfo> View::getEvents() const
{
  return{
    EP_MAKE_EVENT(dirty, "View dirty event"),
    EP_MAKE_EVENT(frameReady, "The next frame has finished rendering"),
    EP_MAKE_EVENT(enabledPickingChanged, "Enable Picking changed"),
    EP_MAKE_EVENT(pickFound, "Pick found"),
    EP_MAKE_EVENT(mousePositionChanged, "Mouse Position changed")
  };
}

void ViewImpl::setScene(SceneRef spNewScene)
{
  if (spScene == spNewScene)
    return;

  if (spScene)
    spScene->dirty.unsubscribe(this, &ViewImpl::onDirty);

  spScene = spNewScene;

  if (spScene)
    spScene->dirty.subscribe(this, &ViewImpl::onDirty);

  onDirty();
}

void ViewImpl::setEnablePicking(bool enable)
{
  if (pickingEnabled != enable)
  {
    pickingEnabled = enable;
    pInstance->enabledPickingChanged.signal(pickingEnabled);
  }
}

void ViewImpl::resize(int width, int height)
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
    onDirty();
}

void ViewImpl::gotoBookmark(String bookmarkName)
{
  if (spScene && spCamera)
  {
    const Bookmark *pBM = spScene->findBookmark(bookmarkName);
    if (pBM)
    {
      spCamera->setMatrix(Double4x4::rotationYPR(pBM->ypr, pBM->position));
      onDirty();
    }
  }
}

bool ViewImpl::inputEvent(const ep::InputEvent &ev)
{
  if (ev.deviceType == InputDevice::Mouse)
  {
    if (ev.eventType == ep::InputEvent::EventType::Move)
    {
      mousePosition.x = (uint32_t)ev.move.xAbsolute;
      mousePosition.y = (uint32_t)ev.move.yAbsolute;
      pInstance->mousePositionChanged.signal(mousePosition);
    }
  }
  else if (ev.deviceType == ep::InputDevice::Keyboard)
  {
    if ((KeyCode)ev.key.key == KeyCode::P && ev.key.state == 0)
      setEnablePicking(!pickingEnabled);
  }

  bool handled = inputEventHook ? inputEventHook(ev) : false;

  if (!handled && spScene)
    handled = spScene->inputEvent(ev);
  if (!handled && spCamera)
    handled = spCamera->viewportInputEvent(ev);
  return handled;
}

void ViewImpl::onDirty()
{
  if (renderWidth == 0 || renderHeight == 0)
  {
    getKernel()->logWarning(1, "Render target has zero size! Unable to render...");
    return;
  }

  if (spScene && spCamera)
  {
    SharedPtr<Renderer> spRenderer = getKernel()->getImpl()->GetRenderer();

    UniquePtr<RenderableView> spRenderView = UniquePtr<RenderableView>::create(spRenderer);

    spRenderView->pRenderEngine = spRenderer->GetRenderEngine();

    spRenderView->spView = ViewRef(pInstance);

    spRenderView->camera = spCamera->getCameraMatrix();
    spRenderView->projection = spCamera->getProjectionMatrix((double)displayWidth / (double)displayHeight);

    spRenderView->clearColor = clearColor;

    spRenderView->spScene = spScene->getRenderScene();

    spRenderView->options = udRenderOptions{ sizeof(udRenderOptions), getRenderableUDFlags(), 0, nullptr, nullptr,
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
      spRenderView->udPickHighlight.nodeIndex = pickHighlightData.highlightIndex;
      spRenderView->udPickHighlight.pModel = pickHighlightData.highlightModel;
      spRenderView->udPickHighlight.color = 0xFFC0C000;
    }

    // if there are ud jobs, we'll need to send it to the UD render thread
    if (spRenderView->spScene->ud.length > 0)
    {
      spRenderView->spColorBuffer = spRenderer->AllocRenderBuffer();
      spRenderView->spColorBuffer->allocate<uint32_t>(Slice<const size_t>{ (size_t)renderWidth, (size_t)renderHeight });
      spRenderView->spDepthBuffer = spRenderer->AllocRenderBuffer();
      spRenderView->spDepthBuffer->allocate<float>(Slice<const size_t>{ (size_t)renderWidth, (size_t)renderHeight });
      spRenderer->AddUDRenderJob(std::move(spRenderView));
    }
    else
      setLatestFrame(std::move(spRenderView));
  }

  // emit the dirty signal
  pInstance->dirty.signal();
}

void ViewImpl::setLatestFrame(UniquePtr<RenderableView> spFrame)
{
  spLatestFrame = spFrame;
  if (spLatestFrame)
  {
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

        pickedPoint = (reinterpret_cast<const UDRenderContext*>(pick.pModel)->matrix * transPos).toVector3();

        pickHighlightData.highlightModel = pick.pModel;
        pickHighlightData.highlightIndex = pick.nodeIndex;

        NodeRef udNodePtr = nullptr;

        for (const UDJob &job : spLatestFrame->spScene->ud)
        {
          const udRenderModel* prModel = reinterpret_cast<const udRenderModel*>(&job.context);

          if (prModel == pick.pModel)
          {
            udNodePtr = job.udNodePtr;
            break;
          }
        }

        EPASSERT(udNodePtr != nullptr, "No node for octree");

        pInstance->pickFound.signal(pickedPoint, udNodePtr);
      }
      else
      {
        pickedPoint = { 0 , 0, 0 };
        pickHighlightData.highlightModel = nullptr;
      }
    }
    pInstance->frameReady.signal();
  }
}

Double3 ViewImpl::screenToNDCPoint(Double2 screenPixel, double z) const
{
  // TODO: this presumes OpenGl clip space
  Dimensions<int> screenSize = getDimensions();

  double glPixelX = double(screenPixel.x) / (screenSize.width * 0.5) - 1.0;
  double glPixelY = -(double(screenPixel.y) / (screenSize.height * 0.5) - 1.0);

  return Double3{ glPixelX, glPixelY, z * 2.0 - 1.0 };
}

Double3 ViewImpl::ndcToScreenPoint(Double3 ndcPoint) const
{
  // TODO: this presumes OpenGl clip space
  Dimensions<int> screenSize = getDimensions();

  double pixelX = (ndcPoint.x * 0.5 + 0.5) * screenSize.width;
  double pixelY = (-ndcPoint.y * 0.5 + 0.5) * screenSize.height;

  return Double3{ pixelX, pixelY, ndcPoint.z * 0.5 + 0.5 };
}

Double3 ViewImpl::unprojectScreenPoint(const Double4x4 &invProj, Double2 screenPixel, double z) const
{
  Double3 ndcPoint = screenToNDCPoint(screenPixel, z);

  Double4 unprojectedPoint = invProj * Double4::create(ndcPoint, 1);
  return unprojectedPoint.toVector3() / unprojectedPoint.w;
}

Double3 ViewImpl::projectToScreenPoint(const Double4x4 &proj, Double3 point) const
{
  Double4 clipPoint = proj * Double4::create(point, 1);
  Double3 ndcPoint = clipPoint.toVector3() / clipPoint.w;

  return ndcToScreenPoint(ndcPoint);
}

DoubleRay3 ViewImpl::screenPointToWorldRay(Double2 screenPoint) const
{
  EPTHROW_IF_NULL(spCamera, Result::InvalidCall, "This function needs the camera to be available");

  Dimensions<int> screenSize = getDimensions();

  Double4x4 viewMat = spCamera->getViewMatrix();
  Double4x4 projMat = spCamera->getProjectionMatrix((double)screenSize.width / (double)screenSize.height);

  Double4x4 VP = projMat * viewMat;
  Double4x4 invVP = Inverse(VP);

  Double2 ndcPoint = screenToNDCPoint(screenPoint).toVector2(); // screen to clip point

  Double4 worldLinePoint1 = invVP * Double4::create(ndcPoint, -1, 1); // start at the near plane
  Double4 worldLinePoint2 = invVP * Double4::create(ndcPoint, 1, 1);  // go to the end plane

  Double3 rayStart = worldLinePoint1.toVector3() / worldLinePoint1.w;
  Double3 rayDirection = worldLinePoint2.toVector3() / worldLinePoint2.w - rayStart;

  return DoubleRay3{ rayStart, rayDirection };
}

} // namespace ep
