#pragma once
#if !defined(_EP_VIEW_HPP)
#define _EP_VIEW_HPP

#include "ep/cpp/component/component.h"
#include "ep/cpp/internal/i/iview.h"

#include "ep/cpp/component/node/camera.h"
#include "ep/cpp/component/scene.h"


namespace ep {

SHARED_CLASS(View);

class View : public Component, public IView
{
  EP_DECLARE_COMPONENT_WITH_IMPL(View, IView, Component, EPKERNEL_PLUGINVERSION, "View desc...")
public:
  void SetScene(SceneRef spScene) override final { pImpl->SetScene(spScene); }
  SceneRef GetScene() const override final { return pImpl->GetScene(); }

  void SetCamera(CameraRef spCamera) override final { pImpl->SetCamera(spCamera); }
  CameraRef GetCamera() const override final { return pImpl->GetCamera(); }

  Dimensions<int> GetDimensions() const override final { return pImpl->GetDimensions(); }
  Dimensions<int> GetRenderDimensions() const override final { return pImpl->GetRenderDimensions(); }
  float GetAspectRatio() const override final { return pImpl->GetAspectRatio(); }

  void SetEnablePicking(bool enable) override final { pImpl->SetEnablePicking(enable); }
  bool GetEnablePicking() const override final { return pImpl->GetEnablePicking(); }

  ScreenPoint GetMousePosition() const override final { return pImpl->GetMousePosition(); }

  void GoToBookmark(String bookmarkName) override { pImpl->GoToBookmark(bookmarkName); }
  void Activate() override { pImpl->Activate(); }
  void Deactivate() override { pImpl->Deactivate(); }
  void Resize(int width, int height) override { pImpl->Resize(width, height); }

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

  Array<const PropertyInfo> GetProperties()
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

  static Array<const MethodInfo> GetMethods()
  {
    return{
      EP_MAKE_METHOD(GoToBookmark, "Move the Camera to the specified Bookmark"),
      EP_MAKE_METHOD(Activate, "Activate the View, e.g. start rendering"),
      EP_MAKE_METHOD(Deactivate, "Deactivate the View, e.g. stop rendering"),
      EP_MAKE_METHOD(Resize, "Resize the View"),
    };
  }

  static Array<const EventInfo> GetEvents()
  {
    return{
      EP_MAKE_EVENT(Dirty, "View dirty event"),
      EP_MAKE_EVENT(FrameReady, "The next frame has finished rendering"),
      EP_MAKE_EVENT(EnabledPickingChanged, "Enable Picking changed"),
      EP_MAKE_EVENT(PickFound, "Pick found"),
      EP_MAKE_EVENT(MousePositionChanged, "Mouse Position changed")
    };
  }
};

} // namespace ep

#endif // _EP_VIEW_HPP
