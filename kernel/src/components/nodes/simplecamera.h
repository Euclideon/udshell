#pragma once
#ifndef EPSIMPLECAMERA_H
#define EPSIMPLECAMERA_H

#include "ep/cpp/component/component.h"
#include "ep/cpp/component/node/camera.h"
#include "ep/cpp/component/node/node.h"
#include "hal/input.h"

namespace ep {

SHARED_CLASS(SimpleCamera);

class SimpleCamera : public Camera
{
  EP_DECLARE_COMPONENT(SimpleCamera, Camera, EPKERNEL_PLUGINVERSION, "SimpleCamera desc...")
public:
  void SetMatrix(const Double4x4 &_matrix) override { pos = _matrix.axis.t.toVector3(); ypr = _matrix.extractYPR(); Camera::SetMatrix(_matrix); }
  void SetPosition(const Double3 &_pos) override { pos = _pos; Camera::SetPosition(_pos); }

  void SetOrientation(const Double3 &_ypr) { ypr = _ypr; Camera::SetMatrix(Double4x4::rotationYPR(ypr.x, ypr.y, ypr.z, pos)); }
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

  bool ViewportInputEvent(const epInputEvent &ev) override;
  bool Update(double timeStep) override;

  SimpleCamera(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);

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
