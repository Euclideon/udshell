#pragma once
#ifndef EPVIEWIMPL_H
#define EPVIEWIMPL_H

#include "ep/cpp/platform.h"
#include "ep/cpp/component/view.h"
#include "ep/cpp/component/scene.h"
#include "ep/cpp/component/node/camera.h"

#include "ep/cpp/kernel.h"
#include "renderscene.h"

#include "udRender.h"

namespace ep {

class ViewImpl : public BaseImpl<View, IView>
{
public:
  ViewImpl(Component *pInstance, Variant::VarMap initParams)
    : ImplSuper(pInstance) {}

  void setScene(SceneRef spScene) override final;
  SceneRef getScene() const override final { return spScene; }

  void setCamera(CameraRef _spCamera) override final { spCamera = _spCamera; onDirty(); }
  CameraRef getCamera() const override final { return spCamera; }

  Dimensions<int> getDimensions() const override final { return{ displayWidth, displayHeight }; }
  Dimensions<int> getRenderDimensions() const override final { return{ renderWidth, renderHeight }; }
  float getAspectRatio() const override final { return (float)displayWidth / (float)displayHeight; }

  void setClearColor(Float4 color) override final { clearColor = color; }
  Float4 getClearColor() const override final  { return clearColor; }

  void setEnablePicking(bool enable) override final;
  bool getEnablePicking() const override final { return pickingEnabled; }

  ScreenPoint getMousePosition() const override final { return mousePosition; }

  void resize(int width, int height) override final;
  void activate() override final { getKernel()->updatePulse.subscribe(Delegate<void(double)>(this, &ViewImpl::update)); }
  void deactivate() override final { getKernel()->updatePulse.unsubscribe(Delegate<void(double)>(this, &ViewImpl::update)); }
  void gotoBookmark(String bookmarkName) override final;

  // TODO: Move this into the layer system once its implemented.
  void setUDRenderFlags(UDRenderFlags flags) override final { udFlags = flags; }
  UDRenderFlags getUDRenderflags() const override final { return udFlags; }

  // TODO: Implement this/expose to the public api
  struct PickResult
  {
    Double3 position;
    udNodeIndex nodeIndex;
  };
  using PickDelegate = Delegate<void(const SharedArray<PickResult>&)>;

  void requestPick(SharedArray<const ScreenPoint> points, const PickDelegate &del)
  {
#if 0 // TODO: enable this once picking with an array of points is supported
    pickRequests.pushBack() = { points, del };
#endif
  }

  // TODO: This is public because we need to access this in the drivers
  // this needs to be refactored or disappear
  RenderableViewRef getRenderableView() const { return spLatestFrame; }

  // TODO: Should this go in the public (non-impl) api? ideally should be protected but is used in the driver...
  bool inputEvent(const ep::InputEvent &ev);

//private:
//  // TODO: REMOVE THESE FRIEND HACKS (IF POSSIBLE)
//  friend class Renderer;
//  friend class KernelImpl;

  ~ViewImpl() { deactivate(); }

  void onDirty();

  void setLatestFrame(UniquePtr<RenderableView> spFrame);
  void update(double timeStep)
  {
    if ((spCamera && spCamera->update(timeStep)) || pickingEnabled)
      onDirty();
  }

  void setInputEventHook(Delegate<bool(ep::InputEvent)> eventHook) override final { inputEventHook = eventHook; }
  void clearInputEventHook() override final { inputEventHook = nullptr; }

  ep::Double3 screenToNDCPoint(Double2 screenPixel, double z = 0.0) const override final;
  Double3 ndcToScreenPoint(Double3 ndcPoint) const override final;
  Double3 unprojectScreenPoint(const Double4x4 &invProj, Double2 screenPixel, double z = 0.0) const override final;
  Double3 projectToScreenPoint(const Double4x4 &proj, Double3 point) const override final;
  DoubleRay3 screenPointToWorldRay(Double2 screenPoint) const override final;

  Double3 pickedPoint = { 0, 0, 0 };

  struct PickHighlightData
  {
    const udRenderModel *highlightModel;
    udNodeIndex highlightIndex;
  };
  PickHighlightData pickHighlightData = { nullptr, ~0ULL };

  SceneRef spScene = nullptr;
  CameraRef spCamera = nullptr;
  RenderableViewRef spLatestFrame = nullptr;

  int displayWidth = 0, displayHeight = 0;
  int renderWidth = 0, renderHeight = 0;
  Float4 clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };

  ScreenPoint mousePosition = { 0, 0 };

  bool pickingEnabled = false;

  UDRenderFlags udFlags;

  // TODO: RequestPick uses this but needs to be implemented
  /*struct PickRequest
  {
    SharedArray<const ScreenPoint> points;
    PickDelegate pickDelegate;
  };

  Array<PickRequest> pickRequests;*/
private:
  udRenderFlags getRenderableUDFlags();
  Delegate<bool(ep::InputEvent)> inputEventHook;
};


inline udRenderFlags ViewImpl::getRenderableUDFlags()
{
  // DoNotRenderCubes is the same bit as udRF_PointCubes but represents the inverse
  return udRenderFlags(udFlags.v ^ UDRenderFlags::DoNotRenderCubes);
}

} // namespace ep

#endif // EPVIEWIMPL_H
