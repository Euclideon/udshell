#pragma once
#ifndef UDCAMERA_H
#define UDCAMERA_H

#include "udComponent.h"
#include "udSceneGraph.h"


class udCamera : public udNode
{
public:

  virtual udResult InputEvent(const udInputEvent &ev) { return udR_Success; }
  virtual udResult Update(double timeStep) { return udR_Success; }
  virtual udResult Render(const udDouble4x4 &mat) { return udR_Success; } // TODO: render a basic camera image

  void GetCameraMatrix(udDouble4x4 *pMatrix) { CalculateWorldMatrix(pMatrix); }
  void GetViewMatrix(udDouble4x4 *pMatrix) { GetCameraMatrix(pMatrix); pMatrix->inverse(); }

  void GetProjectionMatrix(float aspectRatio, udDouble4x4 *pMatrix);

  void SetPerspective(float fovY) { bOrtho = false; this->fovY = fovY;  }
  void SetOrtho(float orthoHeight) { bOrtho = true; this->orthoHeight = orthoHeight;  }
  void SetDepthPlanes(float zNear, float zFar) { this->zNear = zNear; this->zFar = zFar; }

  static const udComponentDesc descriptor;

protected:
  bool bOrtho = false;
  float fovY = UD_DEG2RADf(60);
  float orthoHeight = 1.f;
  float zNear = 0.1f;
  float zFar = 1000.f;

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
  static udComponent *CreateInstance(udComponentDesc *pType, udKernel *pKernel, udRCString uid, udInitParams initParams);

  virtual void SetMatrix(const udDouble4x4 &matrix) { pos = matrix.axis.t.toVector3(); ypr = matrix.extractYPR(); udCamera::SetMatrix(matrix); }
  virtual void SetPosition(const udDouble3 &pos) { this->pos = pos; udCamera::SetPosition(pos); }

  void SetOrientation(udDouble3 _ypr) { ypr = _ypr; matrix = udDouble4x4::rotationYPR(ypr.x, ypr.y, ypr.z, pos); }
  void SetSpeed(double speed) { this->speed = speed; }

  void InvertYAxis(bool bInvert) { yInvert = bInvert ? -1.0 : 1.0; }
  void HelicopterMode(bool bEnable) { bHelicopter = bEnable; }

  static const udComponentDesc descriptor;

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
