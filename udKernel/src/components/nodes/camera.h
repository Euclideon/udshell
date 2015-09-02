#pragma once
#ifndef UDCAMERA_H
#define UDCAMERA_H

#include "../component.h"
#include "node.h"

namespace ud
{

PROTOTYPE_COMPONENT(Camera);
PROTOTYPE_COMPONENT(SimpleCamera);

class Camera : public Node
{
public:
  UD_COMPONENT(Camera);

  udDouble4x4 GetCameraMatrix() const { udDouble4x4 m; CalculateWorldMatrix(&m); return m; }
  udDouble4x4 GetViewMatrix() const { return GetCameraMatrix().inverse(); }

  void GetProjectionMatrix(double aspectRatio, udDouble4x4 *pMatrix) const;

  void SetPerspective(double fovY) { bOrtho = false; this->fovY = fovY; }
  void SetOrtho(double orthoHeight) { bOrtho = true; this->orthoHeight = orthoHeight; }
  void SetDepthPlanes(double zNear, double zFar) { this->zNear = zNear; this->zFar = zFar; }

protected:
  friend class View;

  bool bOrtho = false;
  double fovY = UD_DEG2RAD(60);
  double orthoHeight = 1.0;
  double zNear = 0.1;
  double zFar = 1000.0;

  Camera(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams)
    : Node(pType, pKernel, uid, initParams) {}

  bool InputEvent(const udInputEvent &ev) override { return false; }
  virtual bool ViewportInputEvent(const udInputEvent &ev) { return false; }
  bool Update(double timeStep) override { return udR_Success; }
};


class SimpleCamera : public Camera
{
public:
  UD_COMPONENT(SimpleCamera);

  static Component *CreateInstance(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams);

  virtual void SetMatrix(const udDouble4x4 &matrix) { pos = matrix.axis.t.toVector3(); ypr = matrix.extractYPR(); Camera::SetMatrix(matrix); }
  virtual void SetPosition(const udDouble3 &pos) { this->pos = pos; Camera::SetPosition(pos); }

  void SetOrientation(const udDouble3 &_ypr) { ypr = _ypr; matrix = udDouble4x4::rotationYPR(ypr.x, ypr.y, ypr.z, pos); }
  void SetSpeed(double speed) { this->speed = speed; }

  void InvertYAxis(bool bInvert) { yInvert = bInvert ? -1.0 : 1.0; }
  void HelicopterMode(bool bEnable) { bHelicopter = bEnable; }

protected:
  udDouble3 pos = udDouble3::zero();
  udDouble3 ypr = udDouble3::zero();

  double yInvert = -1.f;
  double speed = 1.f;

  bool bHelicopter = false;

  enum class Keys
  {
    Up, Down, Left, Right, Elevate, Descend, Boost, Max
  };
  char keyState[(int)Keys::Max];

  SimpleCamera(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams)
    : Camera(pType, pKernel, uid, initParams)
  {
    memset(keyState, 0, sizeof(keyState));
  }

  bool ViewportInputEvent(const udInputEvent &ev) override;
  bool Update(double timeStep) override;
};

} // namespace ud

#endif
