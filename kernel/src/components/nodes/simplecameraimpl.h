#pragma once
#ifndef EPSIMPLECAMERAIMPL_H
#define EPSIMPLECAMERAIMPL_H

#include "ep/cpp/component/node/simplecamera.h"
#include "ep/cpp/internal/i/isimplecamera.h"

#include "ep/cpp/component/component.h"
#include "ep/cpp/component/node/camera.h"
#include "ep/cpp/component/node/node.h"
#include "hal/input.h"

namespace ep {

class SimpleCameraImpl : public BaseImpl<SimpleCamera, ISimpleCamera>
{
public:
  SimpleCameraImpl(Component *pInstance, Variant::VarMap initParams);

  void SetMatrix(const Double4x4 &_matrix) override final { pos = _matrix.axis.t.toVector3(); ypr = _matrix.extractYPR(); pInstance->Super::SetMatrix(_matrix); }
  void SetPosition(const Double3 &_pos) override final { pos = _pos; pInstance->Super::SetPosition(_pos); }

  void SetOrientation(const Double3 &_ypr) override final { ypr = _ypr; pInstance->Super::SetMatrix(Double4x4::rotationYPR(ypr.x, ypr.y, ypr.z, pos)); }
  void SetSpeed(double _speed) override final { speed = _speed; }

  void SetInvertedYAxis(bool bInvert) override final { invertedYAxis = bInvert ? -1.0 : 1.0; }
  bool GetInvertedYAxis() const override final { return invertedYAxis == -1.0; }
  void SetHelicopterMode(bool bEnable) override final { helicopterMode = bEnable; }
  bool GetHelicopterMode() const override final { return helicopterMode; }

  Variant Save() const override final;

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

  bool ViewportInputEvent(const InputEvent &ev) override final;
  bool Update(double timeStep) override final;
};

} // namespace ep

#endif
