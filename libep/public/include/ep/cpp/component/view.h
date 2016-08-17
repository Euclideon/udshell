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
  //! \see getScene, setCamera
  void setScene(SceneRef spScene) { pImpl->setScene(spScene); }

  //! Gets the Scene associated with the view.
  //! \return The scene currently associated with the view.
  //! \see setScene, getCamera
  SceneRef getScene() const { return pImpl->getScene(); }

  //! Sets a Camera for the view.
  //! \param spCamera The camera to associate with this view.
  //! \return None.
  //! \see getCamera, setScene
  void setCamera(CameraRef spCamera) { pImpl->setCamera(spCamera); }

  //! Gets the Camera associated with the view.
  //! \return The camera currently associated with the view.
  //! \see setCamera, getScene
  CameraRef getCamera() const { return pImpl->getCamera(); }

  Dimensions<int> getDimensions() const { return pImpl->getDimensions(); }
  Dimensions<int> getRenderDimensions() const { return pImpl->getRenderDimensions(); }

  float getAspectRatio() const { return pImpl->getAspectRatio(); }

  //! Sets the clear color for the view.
  //! \param color The clear color for the view.
  //! \return None.
  void setClearColor(Float4 color) { pImpl->setClearColor(color); }

  //! Gets the clear color for the view.
  //! \return The clear color for the view.
  Float4 getClearColor() const { return pImpl->getClearColor(); }

  void setEnablePicking(bool enable) { pImpl->setEnablePicking(enable); }
  bool getEnablePicking() const { return pImpl->getEnablePicking(); }

  ScreenPoint getMousePosition() const { return pImpl->getMousePosition(); }

  void gotoBookmark(String bookmarkName) { pImpl->gotoBookmark(bookmarkName); }
  void activate() { pImpl->activate(); }
  void deactivate() { pImpl->deactivate(); }
  void resize(int width, int height) { pImpl->resize(width, height); }

  // TODO: Move this into the layer system once its implemented.
  void setUDRenderFlags(UDRenderFlags flags) { pImpl->setUDRenderFlags(flags); }
  UDRenderFlags getUDRenderflags() const { return pImpl->getUDRenderflags(); }

  //! Sets an input event hook that is called prior to normal handling of input events. If a hook already exists, it will be replaced.
  //! \param inputEventHook A delegate to call for each input event. Delegate should return \c true if the event was handled and should not be passed on to the default input handler.
  //! \return None.
  //! \see clearInputEventHook
  void setInputEventHook(Delegate<bool(InputEvent)> inputEventHook) { pImpl->setInputEventHook(inputEventHook); }

  //! Clears the input event hook if one exists.
  //! \return None.
  //! \see setInputEventHook
  void clearInputEventHook() { pImpl->clearInputEventHook(); }

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
  //! \see orthoClipToScreenPoint, unprojectScreenPoint, projectToScreenPoint, screenPointToWorldRay
  Double3 screenToNDCPoint(Double2 screenPixel, double z = 0.0) const { return pImpl->screenToNDCPoint(screenPixel, z); }

  //! Transform an NDC (Normalized Device Coordinate) point to screen space.
  //! \param ndcPoint NDC point to transform.
  //! \return \a ndcPoint transformed to screen space.
  //! \see screenToOrthoClipPoint, projectToScreenPoint, unprojectScreenPoint, screenPointToWorldRay
  Double3 ndcToScreenPoint(Double3 ndcPoint) const { return pImpl->ndcToScreenPoint(ndcPoint); }

  //! Unproject screen space point to world space using the supplied projection matrix.
  //! \param invProj The inverse projection matrix. User may pass a ViewProjection matrix to transform the point to world space.
  //! \param screenPixel The pixel to transform.
  //! \param z The normalised depth of the point. [0..1]
  //! \return \a screenPixel unprojected.
  //! \remarks Note: The matrix supplied must be the inverse of the projection matrix that would project from world to screen space.
  //! \see projectToScreenPoint, screenPointToWorldRay
  Double3 unprojectScreenPoint(const Double4x4 &invProj, Double2 screenPixel, double z = 0.0) const { return pImpl->unprojectScreenPoint(invProj, screenPixel, z); }

  //! Project a world space point to screen space using the supplied projection matrix.
  //! \param proj The projection matrix. User may pass a ViewProjection matrix to transform from a world space point.
  //! \param point World space point to transform.
  //! \return \a point projected to screen space.
  //! \see unprojectScreenPoint, screenPointToWorldRay
  Double3 projectToScreenPoint(const Double4x4 &proj, Double3 point) const { return pImpl->projectToScreenPoint(proj, point); }

  //! Transform screen space point to a world space ray. The returned ray spans the view frustum from the near plane to the far plane.
  //! \param screenPixel Screen space point of the way.
  //! \return \a screenPixel as a world space ray.
  //! \remarks Note: This function throws if \c Camera is \c nullptr.
  //! \see screenToClipPoint, screenToWorldPoint
  DoubleRay3 screenPointToWorldRay(Double2 screenPixel) const { return pImpl->screenPointToWorldRay(screenPixel); }

  Event<> dirty;
  Event<> frameReady;
  Event<bool> enabledPickingChanged;
  Event<Double3, NodeRef&> pickFound;
  Event<ScreenPoint> mousePositionChanged;

protected:
  View(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Component(pType, pKernel, uid, initParams)
  {
    pImpl = createImpl(initParams);
  }

private:
  Array<const PropertyInfo> getProperties() const;
  Array<const MethodInfo> getMethods() const;
  Array<const EventInfo> getEvents() const;
};

} // namespace ep

#endif // _EP_VIEW_HPP
