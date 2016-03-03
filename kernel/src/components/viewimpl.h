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
    : ImplSuper(pInstance)
  {
    memset(&options, 0, sizeof(options));
  }

  void SetScene(SceneRef spScene) override final;
  SceneRef GetScene() const override final { return spScene; }

  void SetCamera(CameraRef _spCamera) override final { spCamera = _spCamera; OnDirty(); }
  CameraRef GetCamera() const override final { return spCamera; }

  Dimensions<int> GetDimensions() const override final { return{ displayWidth, displayHeight }; }
  Dimensions<int> GetRenderDimensions() const override final { return{ renderWidth, renderHeight }; }
  float GetAspectRatio() const override final { return (float)displayWidth / (float)displayHeight; }

  // TODO: Re-evaluate these functions when we figure out how we're handling ud in the public interface
  void SetRenderOptions(const udRenderOptions &_options) { options = _options; }
  const udRenderOptions &GetRenderOptions() const { return options; }

  void SetEnablePicking(bool enable) override final;
  bool GetEnablePicking() const override final { return pickingEnabled; }

  ScreenPoint GetMousePosition() const override final { return mousePosition; }

  void Resize(int width, int height) override final;
  void Activate() override final { GetKernel()->UpdatePulse.Subscribe(Delegate<void(double)>(this, &ViewImpl::Update)); }
  void Deactivate() override final { GetKernel()->UpdatePulse.Unsubscribe(Delegate<void(double)>(this, &ViewImpl::Update)); }
  void GoToBookmark(String bookmarkName) override final;

  // TODO: Move this into the layer system once its implemented.
  void SetUDRenderFlags(UDRenderFlags flags) override final { renderFlags = flags; }
  UDRenderFlags GetUDRenderflags() const override final { return renderFlags; }

  // TODO: Implement this/expose to the public api
  struct PickResult
  {
    Double3 position;
    udNodeIndex nodeIndex;
  };
  using PickDelegate = Delegate<void(const SharedArray<PickResult>&)>;

  void RequestPick(SharedArray<const ScreenPoint> points, const PickDelegate &del)
  {
#if 0 // TODO: enable this once picking with an array of points is supported
    pickRequests.pushBack() = { points, del };
#endif
  }

  // TODO: This is public because we need to access this in the drivers
  // this needs to be refactored or disappear
  RenderableViewRef GetRenderableView() const { return spLatestFrame; }

  // TODO: Should this go in the public (non-impl) api? ideally should be protected but is used in the driver...
  bool InputEvent(const epInputEvent &ev);

//private:
//  // TODO: REMOVE THESE FRIEND HACKS (IF POSSIBLE)
//  friend class Renderer;
//  friend class KernelImpl;

  ~ViewImpl() { Deactivate(); }

  void OnDirty();

  void SetLatestFrame(UniquePtr<RenderableView> spFrame);
  void Update(double timeStep)
  {
    if ((spCamera && spCamera->Update(timeStep)) || pickingEnabled)
      OnDirty();
  }

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

  ScreenPoint mousePosition = { 0, 0 };

  // --- possibly derived stuff? ---
  udRenderOptions options;

  bool pickingEnabled = false;

  UDRenderFlags renderFlags;

  // TODO: RequestPick uses this but needs to be implemented
  /*struct PickRequest
  {
    SharedArray<const ScreenPoint> points;
    PickDelegate pickDelegate;
  };

  Array<PickRequest> pickRequests;*/
};

} // namespace ep

#endif // EPVIEWIMPL_H
