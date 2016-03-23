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

  static Array<const PropertyInfo> GetProperties()
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
