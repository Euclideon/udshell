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

  //////////////////////////////////////////////////////////////////////////
  // Transformation Spaces:
  // World space -> Clip space   (worldPointvec * WVP)
  // Clip space  -> NDC space    (clipPoint /= clipPoint.w)
  // NDC space   -> Screen space (NDCToScreenPoint(viewportPoint))
  //////////////////////////////////////////////////////////////////////////

  //! Transform a screen pixel to NDC (Normalized Device Coordinate) space.
  //! \param screenPixel The screen coordinate relative to the viewport. { [0..width-1], [0..height-1] }
  //! \param z The normalised depth of the point. [0..1]
  //! \return { \a screenPixel.xy, \a z } in NDC space.
  //! \see OrthoClipToScreenPoint, UnprojectScreenPoint, ProjectToScreenPoint, ScreenPointToWorldRay
  Double3 ScreenToNDCPoint(Double2 screenPixel, double z = 0.0) const { return pImpl->ScreenToNDCPoint(screenPixel, z); }

  //! Transform an NDC (Normalized Device Coordinate) point to screen space.
  //! \param ndcPoint NDC point to transform.
  //! \return \a ndcPoint transformed to screen space.
  //! \see ScreenToOrthoClipPoint, ProjectToScreenPoint, UnprojectScreenPoint, ScreenPointToWorldRay
  Double3 NDCToScreenPoint(Double3 ndcPoint) const { return pImpl->NDCToScreenPoint(ndcPoint); }

  //! Unproject screen space point to world space using the supplied projection matrix.
  //! \param invProj The inverse projection matrix. User may pass a ViewProjection matrix to transform the point to world space.
  //! \param screenPixel The pixel to transform.
  //! \param z The normalised depth of the point. [0..1]
  //! \return \a screenPixel unprojected.
  //! \remarks Note: The matrix supplied must be the inverse of the projection matrix that would project from world to screen space.
  //! \see ProjectToScreenPoint, ScreenPointToWorldRay
  Double3 UnprojectScreenPoint(const Double4x4 &invProj, Double2 screenPixel, double z = 0.0) const { return pImpl->UnprojectScreenPoint(invProj, screenPixel, z); }

  //! Project a world space point to screen space using the supplied projection matrix.
  //! \param proj The projection matrix. User may pass a ViewProjection matrix to transform from a world space point.
  //! \param point World space point to transform.
  //! \return \a point projected to screen space.
  //! \see UnprojectScreenPoint, ScreenPointToWorldRay
  Double3 ProjectToScreenPoint(const Double4x4 &proj, Double3 point) const { return pImpl->ProjectToScreenPoint(proj, point); }

  //! Transform screen space point to a world space ray. The returned ray spans the view frustum from the near plane to the far plane.
  //! \param screenPixel Screen space point of the way.
  //! \return \a screenPixel as a world space ray.
  //! \remarks Note: This function throws if \c Camera is \c nullptr.
  //! \see ScreenToClipPoint, ScreenToWorldPoint
  DoubleRay3 ScreenPointToWorldRay(Double2 screenPixel) const { return pImpl->ScreenPointToWorldRay(screenPixel); }

  Event<> Dirty;
  Event<> FrameReady;
  Event<bool> EnabledPickingChanged;
  Event<Double3, NodeRef&> PickFound;
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
