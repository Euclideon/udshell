#pragma once
#ifndef EPCAMERAIMPL_H
#define EPCAMERAIMPL_H

#include "ep/cpp/component/component.h"
#include "ep/cpp/component/node/node.h"
#include "ep/cpp/component/node/camera.h"
#include "ep/cpp/internal/i/icamera.h"
#include "hal/input.h"

namespace ep {

class CameraImpl : public BaseImpl<Camera, ICamera>
{
public:
  CameraImpl(Component *pInstance, Variant::VarMap initParams);

  Double4x4 GetCameraMatrix() const override final { Double4x4 m; pInstance->Super::CalculateWorldMatrix(&m); return m; }
  Double4x4 GetViewMatrix() const override final { return GetCameraMatrix().inverse(); }

  Double4x4 GetProjectionMatrix(double aspectRatio) const override final;

  void SetPerspective(double _fovY) override final { bOrtho = false; fovY = _fovY; }
  void SetOrtho(double _orthoHeight) override final { bOrtho = true; orthoHeight = _orthoHeight; }

  double GetFovY() const override final { return fovY; }
  double GetOrthoHeight() const override final { return orthoHeight; }
  bool IsOrtho() const override final { return bOrtho; }

  void SetDepthPlanes(double _zNear, double _zFar) override final { zNear = _zNear; zFar = _zFar; }
  void SetNearPlane(double _zNear) override final { zNear = _zNear; }
  double GetNearPlane() const override final { return zNear; }
  void SetFarPlane(double _zFar) override final { zFar = _zFar; }
  double GetFarPlane() const override final { return zFar; }

  Variant Save() const override final;

protected:
  bool bOrtho = false;
  double fovY = EP_DEG2RAD(60);
  double orthoHeight = 1.0;
  double zNear = 0.1;
  double zFar = 1000.0;

  bool InputEvent(const ep::InputEvent &ev) override final { return false; }
  bool ViewportInputEvent(const ep::InputEvent &ev) override final { return false; }
  bool Update(double timeStep) override final { return false; }
};

} // namespace ep

#endif
