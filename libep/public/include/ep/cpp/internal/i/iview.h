#pragma once
#if !defined(_EP_IVIEW_HPP)
#define _EP_IVIEW_HPP

#include "ep/cpp/rect.h"
#include "ep/cpp/render.h"

namespace ep {

SHARED_CLASS(Scene);
SHARED_CLASS(Camera);

class IView
{
public:
  using ScreenPoint = Vector2<int>;

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

};

} // namespace ep

#endif // _EP_IVIEW_HPP
