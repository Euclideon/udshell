#pragma once
#ifndef EPCAMERA_H
#define EPCAMERA_H

#include "ep/cpp/component/component.h"
#include "node.h"
#include "hal/input.h"

namespace ep {

SHARED_CLASS(Camera);
SHARED_CLASS(SimpleCamera);

class Camera : public Node
{
  EP_DECLARE_COMPONENT(Camera, Node, EPKERNEL_PLUGINVERSION, "Camera desc...")
public:

  Double4x4 GetCameraMatrix() const { Double4x4 m; CalculateWorldMatrix(&m); return m; }
  Double4x4 GetViewMatrix() const { return GetCameraMatrix().inverse(); }

  // TODO: Implement this?
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

  Camera(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Node(pType, pKernel, uid, initParams)
  {
    const Variant *perspParam = initParams.Get("perspective");
    if (perspParam)
      SetPerspective(perspParam->asFloat());

    const Variant *orthoParam = initParams.Get("ortho");
    if (orthoParam)
      SetOrtho(orthoParam->asFloat());

    const Variant *paramDepthPlanes = initParams.Get("depthplanes");
    if (paramDepthPlanes)
    {
      auto depthPlanesArray = paramDepthPlanes->as<Array<double, 2>>();
      if (depthPlanesArray.length == 2)
        SetDepthPlanes(depthPlanesArray[0], depthPlanesArray[1]);
    }
  }

  bool InputEvent(const epInputEvent &ev) override { return false; }
  virtual bool ViewportInputEvent(const epInputEvent &ev) { return false; }
  bool Update(double timeStep) override { return epR_Success; }

  static Array<const PropertyInfo> GetProperties()
  {
    return{
      EP_MAKE_PROPERTY_RO(CameraMatrix, "Position of camera", nullptr, 0),
      EP_MAKE_PROPERTY_RO(ViewMatrix, "Position of camera", nullptr, 0),
    };
  }
  static Array<const MethodInfo> GetMethods()
  {
    return{
      EP_MAKE_METHOD(SetDepthPlanes, "Set the near and far depth planes:\n  setdepthplanes(near, far)"),
      EP_MAKE_METHOD(SetOrtho, "Set the projection mode to Orthographic with given ortho height"),
      EP_MAKE_METHOD(SetPerspective, "Set the projection mode to Perspective with given field of view"),
    };
  }
};


class SimpleCamera : public Camera
{
  EP_DECLARE_COMPONENT(SimpleCamera, Camera, EPKERNEL_PLUGINVERSION, "SimpleCamera desc...")
public:
  void SetMatrix(const Double4x4 &_matrix) override { pos = _matrix.axis.t.toVector3(); ypr = _matrix.extractYPR(); Camera::SetMatrix(_matrix); }
  void SetPosition(const Double3 &_pos) override { pos = _pos; Camera::SetPosition(_pos); }

  void SetOrientation(const Double3 &_ypr) { ypr = _ypr; matrix = Double4x4::rotationYPR(ypr.x, ypr.y, ypr.z, pos); }
  void SetSpeed(double _speed) { speed = _speed; }

  void SetInvertedYAxis(bool bInvert) { invertedYAxis = bInvert ? -1.0 : 1.0; }
  bool GetInvertedYAxis() const { return invertedYAxis == -1.0; }
  void SetHelicopterMode(bool bEnable) { helicopterMode = bEnable; }
  bool GetHelicopterMode() const { return helicopterMode; }

  Variant Save() const override;

  Event<Double3, Double3> Changed;
protected:
  Double3 pos = Double3::zero();
  Double3 ypr = Double3::zero();

  double invertedYAxis = -1.f;
  double speed = 1.f;

  bool helicopterMode = false;
  bool stateChanged = false;

  enum class Keys
  {                                                 // TODO: Stitch up Rolls
    Up, Down, Left, Right, Elevate, Descend, Boost, Roll_Left, Roll_Right, Key_1, Key_2, Max
  };
  char keyState[(int)Keys::Max];


  struct Mouse
  {
    Mouse() { memset(buttons, 0, sizeof(buttons)); }
    enum
    {
      Left = 0, Middle, Right, Max
    };
    char stateChanged[Mouse::Max];
    char buttons[Mouse::Max];
    Float2 absolute = { 0, 0 };
    Float2 prevAbsolute = { 0, 0 };
    Float2 delta = { 0, 0 };
  } mouse;

  SimpleCamera(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Camera(pType, pKernel, uid, initParams)
  {
    memset(keyState, 0, sizeof(keyState));
    const Variant *paramPos = initParams.Get("position");
    if (paramPos)
    {
      auto posArray = paramPos->as<Array<double, 3>>();
      if (posArray.length == 3)
      {
        Double3 position = Double3::create(posArray[0], posArray[1], posArray[2]);
        SetPosition(position);
      }
    }

    const Variant *paramOrientation = initParams.Get("orientation");
    if (paramOrientation)
    {
      auto oriArray = paramOrientation->as<Array<double, 3>>();
      if (oriArray.length == 3)
      {
        Double3 orientation = Double3::create(oriArray[0], oriArray[1], oriArray[2]);
        SetOrientation(orientation);
      }
    }

    const Variant *paramMat = initParams.Get("matrix");
    if (paramMat)
    {
      Array<double> matArray = paramMat->as<Array<double>>();
      if (matArray.length == 16)
        SetMatrix(Double4x4::create(matArray.ptr));
    }

    const Variant *paramSpeed = initParams.Get("speed");
    if (paramSpeed)
      SetSpeed(paramSpeed->asFloat());

    const Variant *paramInvert = initParams.Get("invertyaxis");
    if (paramInvert)
      SetInvertedYAxis(paramInvert->asBool());

    const Variant *paramHeli = initParams.Get("helicoptermode");
    if (paramHeli)
      SetHelicopterMode(paramHeli->asBool());
  }

  bool ViewportInputEvent(const epInputEvent &ev) override;
  bool Update(double timeStep) override;

  static Array<const PropertyInfo> GetProperties()
  {
    return{
      EP_MAKE_PROPERTY_WO(Matrix, "Local matrix", nullptr, 0),
      EP_MAKE_PROPERTY_WO(Position, "Local position", nullptr, 0),
      EP_MAKE_PROPERTY_WO(Orientation, "Camera orientation (YPR)", nullptr, 0),
      EP_MAKE_PROPERTY_WO(Speed, "Camera speed", nullptr, 0),
      EP_MAKE_PROPERTY(HelicopterMode, "Helicopter Mode", nullptr, 0),
      EP_MAKE_PROPERTY(InvertedYAxis, "InvertYAxis", nullptr, 0),
    };
  }
  static Array<const EventInfo> GetEvents()
  {
    return {
      EP_MAKE_EVENT(Changed, "The camera changed")
    };
  }
};

} // namespace ep

#endif
