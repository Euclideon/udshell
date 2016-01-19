#pragma once
#if !defined(_EP_ISIMPLECAMERA_HPP)
#define _EP_ISIMPLECAMERA_HPP

#include "ep/c/input.h"

namespace ep {

class ISimpleCamera
{
public:
  virtual void SetMatrix(const Double4x4 &_matrix) = 0;
  virtual void SetPosition(const Double3 &_pos) = 0;

  virtual void SetOrientation(const Double3 &_ypr) = 0;
  virtual void SetSpeed(double _speed) = 0;

  virtual void SetInvertedYAxis(bool bInvert) = 0;
  virtual bool GetInvertedYAxis() const = 0;
  virtual void SetHelicopterMode(bool bEnable) = 0;
  virtual bool GetHelicopterMode() const = 0;

  virtual Variant Save() const = 0;

protected:
  virtual bool ViewportInputEvent(const epInputEvent &ev) = 0;
  virtual bool Update(double timeStep) = 0;
};

} // namespace ep

#endif
