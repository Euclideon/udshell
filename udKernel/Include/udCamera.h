#pragma once
#ifndef UDCAMERA_H
#define UDCAMERA_H

#include "udComponent.h"
#include "udNode.h"

PROTOTYPE_COMPONENT(udCamera);
PROTOTYPE_COMPONENT(udSimpleCamera);


class udCamera : public udNode
{
public:
  UD_COMPONENT(udCamera);

  virtual udResult InputEvent(const udInputEvent &ev) { return udR_Success; }
  virtual udResult Update(double timeStep) { return udR_Success; }

  udDouble4x4 GetCameraMatrix() const { udDouble4x4 m; CalculateWorldMatrix(&m); return m; }
  udDouble4x4 GetViewMatrix() const { return GetCameraMatrix().inverse(); }

  void GetProjectionMatrix(double aspectRatio, udDouble4x4 *pMatrix) const;

  void SetPerspective(double fovY) { bOrtho = false; this->fovY = fovY; }
  void SetOrtho(double orthoHeight) { bOrtho = true; this->orthoHeight = orthoHeight; }
  void SetDepthPlanes(double zNear, double zFar) { this->zNear = zNear; this->zFar = zFar; }

protected:
  bool bOrtho = false;
  double fovY = UD_DEG2RAD(60);
  double orthoHeight = 1.0;
  double zNear = 0.1;
  double zFar = 1000.0;

protected:
  udCamera(const udComponentDesc *pType, udKernel *pKernel, udRCString uid, udInitParams initParams)
    : udNode(pType, pKernel, uid, initParams) {}
  virtual ~udCamera() {}

  static udComponent *Create(const udComponentDesc *pType, udKernel *pKernel, udRCString uid, udInitParams initParams)
  {
    return udNew(udCamera, pType, pKernel, uid, initParams);
  }
};


class udSimpleCamera : public udCamera
{
public:
  UD_COMPONENT(udSimpleCamera);

  static udComponent *CreateInstance(udComponentDesc *pType, udKernel *pKernel, udRCString uid, udInitParams initParams);

  virtual void SetMatrix(const udDouble4x4 &matrix) { pos = matrix.axis.t.toVector3(); ypr = matrix.extractYPR(); udCamera::SetMatrix(matrix); }
  virtual void SetPosition(const udDouble3 &pos) { this->pos = pos; udCamera::SetPosition(pos); }

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

  virtual udResult InputEvent(const udInputEvent &ev); // Why is this protected?
  virtual udResult Update(double timeStep);

  udSimpleCamera(const udComponentDesc *pType, udKernel *pKernel, udRCString uid, udInitParams initParams)
    : udCamera(pType, pKernel, uid, initParams) {}
  virtual ~udSimpleCamera() {}

  static udComponent *Create(const udComponentDesc *pType, udKernel *pKernel, udRCString uid, udInitParams initParams)
  {
    return udNew(udSimpleCamera, pType, pKernel, uid, initParams);
  }
};

#endif
