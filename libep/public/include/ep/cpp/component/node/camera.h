#pragma once
#ifndef EP_CAMERA_H
#define EP_CAMERA_H

#include "ep/cpp/component/node/node.h"
#include "ep/cpp/internal/i/icamera.h"
#include "ep/c/input.h"

namespace ep {

SHARED_CLASS(Camera);
SHARED_CLASS(View);

class Camera : public Node
{
  EP_DECLARE_COMPONENT_WITH_IMPL(ep, Camera, ICamera, Node, EPKERNEL_PLUGINVERSION, "Camera desc...", 0)

public:
  Double4x4 GetCameraMatrix() const { return pImpl->GetCameraMatrix(); }
  Double4x4 GetViewMatrix() const { return pImpl->GetViewMatrix(); }

  void GetProjectionMatrix(double aspectRatio, Double4x4 *pMatrix) const { pImpl->GetProjectionMatrix(aspectRatio, pMatrix); }

  void SetPerspective(double _fovY) { pImpl->SetPerspective(_fovY); }
  void SetOrtho(double _orthoHeight) { pImpl->SetOrtho(_orthoHeight); }

  double GetFovY() const { return pImpl->GetFovY(); }
  double GetOrthoHeight() const { return pImpl->GetOrthoHeight(); }
  bool IsOrtho() const { return pImpl->IsOrtho(); }

  void SetDepthPlanes(double _zNear, double _zFar) { pImpl->SetDepthPlanes(_zNear, _zFar); }
  void SetNearPlane(double _zNear) { pImpl->SetNearPlane(_zNear); }
  double GetNearPlane() const { return pImpl->GetNearPlane(); }
  void SetFarPlane(double _zFar) { pImpl->SetFarPlane(_zFar); }
  double GetFarPlane() const { return pImpl->GetFarPlane(); }

  Variant Save() const override { return pImpl->Save(); }

protected:
  EP_FRIENDS_WITH_IMPL(View);

  Camera(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Node(pType, pKernel, uid, initParams)
  {
    pImpl = CreateImpl(initParams);
  }

  virtual bool ViewportInputEvent(const epInputEvent &ev) { return pImpl->ViewportInputEvent(ev); }

  bool InputEvent(const epInputEvent &ev) override { return pImpl->InputEvent(ev); }
  bool Update(double timeStep) override { return pImpl->Update(timeStep); }

private:
  Array<const PropertyInfo> GetProperties() const;
  Array<const MethodInfo> GetMethods() const;
};

} // namespace ep

#endif
