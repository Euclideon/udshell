#pragma once
#if !defined(_EP_VIEW_HPP)
#define _EP_VIEW_HPP

#include "ep/cpp/component/component.h"
#include "ep/cpp/internal/i/iview.h"

#include "ep/cpp/component/node/camera.h"
#include "ep/cpp/component/scene.h"


namespace ep {

SHARED_CLASS(View);

class View : public Component
{
  EP_DECLARE_COMPONENT_WITH_IMPL(View, IView, Component, EPKERNEL_PLUGINVERSION, "View desc...", 0)
public:
  void SetScene(SceneRef spScene) { pImpl->SetScene(spScene); }
  SceneRef GetScene() const { return pImpl->GetScene(); }

  void SetCamera(CameraRef spCamera) { pImpl->SetCamera(spCamera); }
  CameraRef GetCamera() const { return pImpl->GetCamera(); }

  Dimensions<int> GetDimensions() const { return pImpl->GetDimensions(); }
  Dimensions<int> GetRenderDimensions() const { return pImpl->GetRenderDimensions(); }
  float GetAspectRatio() const { return pImpl->GetAspectRatio(); }

  void SetEnablePicking(bool enable) { pImpl->SetEnablePicking(enable); }
  bool GetEnablePicking() const { return pImpl->GetEnablePicking(); }

  ScreenPoint GetMousePosition() const { return pImpl->GetMousePosition(); }

  void GoToBookmark(String bookmarkName) { pImpl->GoToBookmark(bookmarkName); }
  void Activate() { pImpl->Activate(); }
  void Deactivate() { pImpl->Deactivate(); }
  void Resize(int width, int height) { pImpl->Resize(width, height); }

  // TODO: Move this into the layer system once its implemented.
  void SetUDRenderFlags(UDRenderFlags flags) { pImpl->SetUDRenderFlags(flags); }
  UDRenderFlags GetUDRenderflags() const { return pImpl->GetUDRenderflags(); }

  Event<> Dirty;
  Event<> FrameReady;
  Event<bool> EnabledPickingChanged;
  Event<Double3> PickFound;
  Event<ScreenPoint> MousePositionChanged;

protected:
  View(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Component(pType, pKernel, uid, initParams)
  {
    pImpl = CreateImpl(initParams);
  }

private:
  Array<const PropertyInfo> GetProperties() const;
  Array<const MethodInfo> GetMethods() const;
  Array<const EventInfo> GetEvents() const;
};

} // namespace ep

#endif // _EP_VIEW_HPP
