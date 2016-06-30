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
  EP_DECLARE_COMPONENT_WITH_IMPL(ep, View, IView, Component, EPKERNEL_PLUGINVERSION, "View desc...", 0)
public:

  //! Sets a Scene for the view.
  //! \param spScene The scene to associate with this view.
  //! \return None.
  //! \see GetScene, SetCamera
  void SetScene(SceneRef spScene) { pImpl->SetScene(spScene); }

  //! Gets the Scene associated with the view.
  //! \return The scene currently associated with the view.
  //! \see SetScene, GetCamera
  SceneRef GetScene() const { return pImpl->GetScene(); }

  //! Sets a Camera for the view.
  //! \param spCamera The camera to associate with this view.
  //! \return None.
  //! \see GetCamera, SetScene
  void SetCamera(CameraRef spCamera) { pImpl->SetCamera(spCamera); }

  //! Gets the Camera associated with the view.
  //! \return The camera currently associated with the view.
  //! \see SetCamera, GetScene
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

  //! Sets an input event hook that is called prior to normal handling of input events. If a hook already exists, it will be replaced.
  //! \param inputEventHook A delegate to call for each input event. Delegate should return \c true if the event was handled and should not be passed on to the default input handler.
  //! \return None.
  //! \see ClearInputEventHook
  void SetInputEventHook(Delegate<bool(InputEvent)> inputEventHook) { pImpl->SetInputEventHook(inputEventHook); }

  //! Clears the input event hook if one exists.
  //! \return None.
  //! \see SetInputEventHook
  void ClearInputEventHook() { pImpl->ClearInputEventHook(); }

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
