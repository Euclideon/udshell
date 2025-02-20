#pragma once
#ifndef EPSIMPLECAMERA_H
#define EPSIMPLECAMERA_H

#include "ep/cpp/component/component.h"
#include "ep/cpp/component/node/camera.h"
#include "ep/cpp/component/node/node.h"
#include "ep/cpp/internal/i/isimplecamera.h"
#include "ep/cpp/input.h"

namespace ep {

SHARED_CLASS(SimpleCamera);

class SimpleCamera : public Camera
{
  EP_DECLARE_COMPONENT_WITH_IMPL(ep, SimpleCamera, ISimpleCamera, Camera, EPKERNEL_PLUGINVERSION, "SimpleCamera desc...", 0)

public:
  void setMatrix(const Double4x4 &_matrix) override { pImpl->SetMatrix(_matrix); }
  void setPosition(const Double3 &_pos) override { pImpl->SetPosition(_pos); }

  void setOrientation(const Double3 &_ypr) { pImpl->SetOrientation(_ypr); }
  void setSpeed(double _speed) { pImpl->SetSpeed(_speed); }

  void setInvertedYAxis(bool bInvert) { pImpl->SetInvertedYAxis(bInvert); }
  bool getInvertedYAxis() const { return pImpl->GetInvertedYAxis(); }
  void setHelicopterMode(bool bEnable) { pImpl->SetHelicopterMode(bEnable); }
  bool getHelicopterMode() const { return pImpl->GetHelicopterMode(); }

  Variant save() const override { return pImpl->Save(); }

  Event<Double3, Double3> repositioned;

protected:
  bool viewportInputEvent(const ep::InputEvent &ev) override { return pImpl->ViewportInputEvent(ev); }
  bool update(double timeStep) override { return pImpl->Update(timeStep); }

  SimpleCamera(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Camera(pType, pKernel, uid, initParams)
  {
    pImpl = createImpl(initParams);
  }

private:
  Array<const PropertyInfo> getProperties() const;
  Array<const EventInfo> getEvents() const;
};

} // namespace ep

#endif
