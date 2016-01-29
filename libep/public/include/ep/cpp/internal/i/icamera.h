#pragma once
#if !defined(_EP_ICAMERA_HPP)
#define _EP_ICAMERA_HPP

#include "ep/cpp/component/component.h"
#include "ep/c/input.h"

namespace ep {

class ICamera
{
public:
  virtual Double4x4 GetCameraMatrix() const = 0;
  virtual Double4x4 GetViewMatrix() const = 0;

  virtual void GetProjectionMatrix(double aspectRatio, Double4x4 *pMatrix) const = 0;

  virtual void SetPerspective(double _fovY) = 0;
  virtual void SetOrtho(double _orthoHeight) = 0;

  virtual void SetDepthPlanes(double _zNear, double _zFar) = 0;
  virtual void SetNearPlane(double _zNear) = 0;
  virtual void SetFarPlane(double _zFar) = 0;

  virtual Variant Save() const = 0;

protected:
  virtual bool InputEvent(const epInputEvent &ev) = 0;
  virtual bool ViewportInputEvent(const epInputEvent &ev) = 0;
  virtual bool Update(double timeStep) = 0;
};

} // namespace ep

#endif
