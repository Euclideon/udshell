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
  virtual void SetScene(SceneRef spScene) = 0;
  virtual SceneRef GetScene() const = 0;

  virtual void SetCamera(CameraRef spCamera) = 0;
  virtual CameraRef GetCamera() const = 0;

  virtual Dimensions<int> GetDimensions() const = 0;
  virtual Dimensions<int> GetRenderDimensions() const = 0;
  virtual float GetAspectRatio() const = 0;

  virtual void SetEnablePicking(bool enable) = 0;
  virtual bool GetEnablePicking() const = 0;

  virtual ScreenPoint GetMousePosition() const = 0;

  virtual void Resize(int width, int height) = 0;
  virtual void Activate() = 0;
  virtual void Deactivate() = 0;
  virtual void GoToBookmark(String bookmarkName) = 0;

  // TODO: Move this into the layer system once its implemented.
  virtual void SetUDRenderFlags(UDRenderFlags flags) = 0;
  virtual UDRenderFlags GetUDRenderflags() const = 0;

  virtual void SetInputEventHook(Delegate<bool(ep::InputEvent)> inputEventHook) = 0;
  virtual void ClearInputEventHook() = 0;

  virtual Double3 ScreenToNDCPoint(Double2 screenPixel, double z) const = 0;
  virtual Double3 NDCToScreenPoint(Double3 ndcPoint) const = 0;
  virtual Double3 UnprojectScreenPoint(const Double4x4 &invProj, Double2 screenPixel, double z) const = 0;
  virtual Double3 ProjectToScreenPoint(const Double4x4 &proj, Double3 point) const = 0;
  virtual DoubleRay3 ScreenPointToWorldRay(Double2 screenPoint) const = 0;
};

} // namespace ep

#endif // _EP_IVIEW_HPP
