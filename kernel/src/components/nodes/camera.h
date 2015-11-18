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

  Variant Save() const override;

protected:
  friend class View;

  bool bOrtho = false;
  double fovY = EP_DEG2RAD(60);
  double orthoHeight = 1.0;
  double zNear = 0.1;
  double zFar = 1000.0;

  Camera(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams)
    : Node(pType, pKernel, uid, initParams)
  {
    const Variant &perspParam = initParams["perspective"];
    if (perspParam.isValid())
      SetPerspective(perspParam.asFloat());

    const Variant &orthoParam = initParams["ortho"];
    if (orthoParam.isValid())
      SetOrtho(orthoParam.asFloat());

    const Variant &paramDepthPlanes = initParams["depthplanes"];
    if (paramDepthPlanes.isValid())
    {
      auto depthPlanesArray = paramDepthPlanes.as<Array<double, 2>>();
      if (depthPlanesArray.length == 2)
        SetDepthPlanes(depthPlanesArray[0], depthPlanesArray[1]);
    }
  }

  bool InputEvent(const epInputEvent &ev) override { return false; }
  virtual bool ViewportInputEvent(const epInputEvent &ev) { return false; }
  bool Update(double timeStep) override { return epR_Success; }
};


class SimpleCamera : public Camera
{
public:
  EP_COMPONENT(SimpleCamera);

  void SetMatrix(const Double4x4 &_matrix) override { pos = _matrix.axis.t.toVector3(); ypr = _matrix.extractYPR(); Camera::SetMatrix(_matrix); }
  void SetPosition(const Double3 &_pos) override { pos = _pos; Camera::SetPosition(_pos); }

  void SetOrientation(const Double3 &_ypr) { ypr = _ypr; matrix = Double4x4::rotationYPR(ypr.x, ypr.y, ypr.z, pos); }
  void SetSpeed(double _speed) { speed = _speed; }

  void InvertYAxis(bool bInvert) { yInvert = bInvert ? -1.0 : 1.0; }
  void HelicopterMode(bool bEnable) { bHelicopter = bEnable; }

  Variant Save() const override;

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

    const Variant &paramPos = initParams["position"];
    if (paramPos.isValid())
    {
      auto posArray = paramPos.as<Array<double, 3>>();
      if (posArray.length == 3)
      {
        Double3 position = Double3::create(posArray[0], posArray[1], posArray[2]);
        SetPosition(position);
      }
    }

    const Variant &paramOrientation = initParams["orientation"];
    if (paramOrientation.isValid())
    {
      auto oriArray = paramOrientation.as<Array<double, 3>>();
      if (oriArray.length == 3)
      {
        Double3 orientation = Double3::create(oriArray[0], oriArray[1], oriArray[2]);
        SetOrientation(orientation);
      }
    }

    const Variant &paramMat = initParams["matrix"];
    if (paramMat.isValid())
    {
      Array<double> matArray = paramMat.as<Array<double>>();
      if (matArray.length == 16)
        SetMatrix(Double4x4::create(matArray.ptr));
    }

    const Variant &paramSpeed = initParams["speed"];
    if (paramSpeed.isValid())
      SetSpeed(paramSpeed.asFloat());

    const Variant &paramInvert = initParams["invertyaxis"];
    if (paramInvert.isValid())
      InvertYAxis(paramInvert.asBool());

    const Variant &paramHeli = initParams["helicoptermode"];
    if (paramHeli.isValid())
      HelicopterMode(paramHeli.asBool());
  }

  bool ViewportInputEvent(const epInputEvent &ev) override;
  bool Update(double timeStep) override;
};

} // namespace ep

#endif
