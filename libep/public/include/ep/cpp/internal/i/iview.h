#pragma once
#if !defined(_EP_IVIEW_HPP)
#define _EP_IVIEW_HPP

#include "ep/cpp/component/component.h"
#include "ep/cpp/rect.h"
#include "ep/cpp/render.h"
#include "ep/cpp/input.h"

namespace ep {

SHARED_CLASS(Scene);
SHARED_CLASS(Camera);

using ScreenPoint = Vector2<int>;

template <typename T>
struct Ray3
{
  Vector3<T> point;    //!< The start point of the ray
  Vector3<T> velocity; //!< velocity represents the direction and distance of the ray from point.
};

using DoubleRay3 = Ray3<double>;

class IView
{
public:
  virtual void setScene(SceneRef spScene) = 0;
  virtual SceneRef getScene() const = 0;

  virtual void setCamera(CameraRef spCamera) = 0;
  virtual CameraRef getCamera() const = 0;

  virtual Dimensions<int> getDimensions() const = 0;
  virtual Dimensions<int> getRenderDimensions() const = 0;
  virtual float getAspectRatio() const = 0;

  virtual void setClearColor(Float4 color) = 0;
  virtual Float4 getClearColor() const = 0;

  virtual void setEnablePicking(bool enable) = 0;
  virtual bool getEnablePicking() const = 0;

  virtual ScreenPoint getMousePosition() const = 0;

  virtual void resize(int width, int height) = 0;
  virtual void activate() = 0;
  virtual void deactivate() = 0;
  virtual void gotoBookmark(String bookmarkName) = 0;

  // TODO: Move this into the layer system once its implemented.
  virtual void setUDRenderFlags(UDRenderFlags flags) = 0;
  virtual UDRenderFlags getUDRenderflags() const = 0;

  virtual void setInputEventHook(Delegate<bool(ep::InputEvent)> inputEventHook) = 0;
  virtual void clearInputEventHook() = 0;

  virtual Double3 screenToNDCPoint(Double2 screenPixel, double z) const = 0;
  virtual Double3 ndcToScreenPoint(Double3 ndcPoint) const = 0;
  virtual Double3 unprojectScreenPoint(const Double4x4 &invProj, Double2 screenPixel, double z) const = 0;
  virtual Double3 projectToScreenPoint(const Double4x4 &proj, Double3 point) const = 0;
  virtual DoubleRay3 screenPointToWorldRay(Double2 screenPoint) const = 0;
};

} // namespace ep

#endif // _EP_IVIEW_HPP
