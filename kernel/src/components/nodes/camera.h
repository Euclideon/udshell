#pragma once
#ifndef EPCAMERA_H
#define EPCAMERA_H

#include "../component.h"
#include "node.h"

namespace ep
{

PROTOTYPE_COMPONENT(Camera);
PROTOTYPE_COMPONENT(SimpleCamera);

class Camera : public Node
{
public:
  EP_COMPONENT(Camera);

  Double4x4 GetCameraMatrix() const { Double4x4 m; CalculateWorldMatrix(&m); return m; }
  Double4x4 GetViewMatrix() const { return GetCameraMatrix().inverse(); }

  void GetProjectionMatrix(double aspectRatio, Double4x4 *pMatrix) const;

  void SetPerspective(double _fovY) { bOrtho = false; fovY = _fovY; }
  void SetOrtho(double _orthoHeight) { bOrtho = true; orthoHeight = _orthoHeight; }
  void SetDepthPlanes(double _zNear, double _zFar) { zNear = _zNear; zFar = _zFar; }

protected:
  friend class View;

  bool bOrtho = false;
  double fovY = EP_DEG2RAD(60);
  double orthoHeight = 1.0;
  double zNear = 0.1;
  double zFar = 1000.0;

  Camera(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams)
    : Node(pType, pKernel, uid, initParams) {}

  bool InputEvent(const epInputEvent &ev) override { return false; }
  virtual bool ViewportInputEvent(const epInputEvent &ev) { return false; }
  bool Update(double timeStep) override { return epR_Success; }
};


class SimpleCamera : public Camera
{
public:
  EP_COMPONENT(SimpleCamera);

  static Component *CreateInstance(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams);

  void SetMatrix(const Double4x4 &_matrix) override { pos = _matrix.axis.t.toVector3(); ypr = _matrix.extractYPR(); Camera::SetMatrix(_matrix); }
  void SetPosition(const Double3 &_pos) override { pos = _pos; Camera::SetPosition(_pos); }

  void SetOrientation(const Double3 &_ypr) { ypr = _ypr; matrix = Double4x4::rotationYPR(ypr.x, ypr.y, ypr.z, pos); }
  void SetSpeed(double _speed) { speed = _speed; }

  void InvertYAxis(bool bInvert) { yInvert = bInvert ? -1.0 : 1.0; }
  void HelicopterMode(bool bEnable) { bHelicopter = bEnable; }

protected:
  Double3 pos = Double3::zero();
  Double3 ypr = Double3::zero();

  double yInvert = -1.f;
  double speed = 1.f;

  bool bHelicopter = false;

  enum class Keys
  {
    Up, Down, Left, Right, Elevate, Descend, Boost, Max
  };
  char keyState[(int)Keys::Max];

  SimpleCamera(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams)
    : Camera(pType, pKernel, uid, initParams)
  {
    memset(keyState, 0, sizeof(keyState));
  }

  bool ViewportInputEvent(const epInputEvent &ev) override;
  bool Update(double timeStep) override;
};

} // namespace ep

#endif
