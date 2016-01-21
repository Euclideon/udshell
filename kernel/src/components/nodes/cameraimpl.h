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

  void GetProjectionMatrix(double aspectRatio, Double4x4 *pMatrix) const override final;

  void SetPerspective(double _fovY) override final { bOrtho = false; fovY = _fovY; }
  void SetOrtho(double _orthoHeight) override final { bOrtho = true; orthoHeight = _orthoHeight; }

  void SetDepthPlanes(double _zNear, double _zFar) override final { zNear = _zNear; zFar = _zFar; }
  void SetNearPlane(double _zNear) override final { zNear = _zNear; }
  void SetFarPlane(double _zFar) override final { zFar = _zFar; }

  Variant Save() const override final;

protected:
  bool bOrtho = false;
  double fovY = EP_DEG2RAD(60);
  double orthoHeight = 1.0;
  double zNear = 0.1;
  double zFar = 1000.0;

  bool InputEvent(const epInputEvent &ev) override final { return false; }
  bool ViewportInputEvent(const epInputEvent &ev) override final { return false; }
  bool Update(double timeStep) override final { return epR_Success; }
};

} // namespace ep

#endif
