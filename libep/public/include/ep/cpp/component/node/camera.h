#pragma once
#ifndef EP_CAMERA_H
#define EP_CAMERA_H

#include "ep/cpp/component/node/node.h"
#include "ep/cpp/internal/i/icamera.h"
#include "ep/c/input.h"

namespace ep {

SHARED_CLASS(Camera);
SHARED_CLASS(View);

class Camera : public Node, public ICamera
{
  EP_DECLARE_COMPONENT_WITH_IMPL(Camera, ICamera, Node, EPKERNEL_PLUGINVERSION, "Camera desc...", 0)

public:
  Double4x4 GetCameraMatrix() const override { return pImpl->GetCameraMatrix(); }
  Double4x4 GetViewMatrix() const override { return pImpl->GetViewMatrix(); }

  void GetProjectionMatrix(double aspectRatio, Double4x4 *pMatrix) const override { pImpl->GetProjectionMatrix(aspectRatio, pMatrix); }

  void SetPerspective(double _fovY) override { pImpl->SetPerspective(_fovY); }
  void SetOrtho(double _orthoHeight) override { pImpl->SetOrtho(_orthoHeight); }

  double GetFovY() const override final { return pImpl->GetFovY(); }
  double GetOrthoHeight() const override final { return pImpl->GetOrthoHeight(); }
  bool IsOrtho() const override final { return pImpl->IsOrtho(); }

  void SetDepthPlanes(double _zNear, double _zFar) override { pImpl->SetDepthPlanes(_zNear, _zFar); }
  void SetNearPlane(double _zNear) override { pImpl->SetNearPlane(_zNear); }
  double GetNearPlane() const override final { return pImpl->GetNearPlane(); }
  void SetFarPlane(double _zFar) override { pImpl->SetFarPlane(_zFar); }
  double GetFarPlane() const override final { return pImpl->GetFarPlane(); }

  Variant Save() const override { return pImpl->Save(); }

protected:
  EP_FRIENDS_WITH_IMPL(View);

  Camera(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Node(pType, pKernel, uid, initParams)
  {
    pImpl = CreateImpl(initParams);
  }

  bool InputEvent(const epInputEvent &ev) override { return pImpl->InputEvent(ev); }
  bool ViewportInputEvent(const epInputEvent &ev) override { return pImpl->ViewportInputEvent(ev); }
  bool Update(double timeStep) override { return pImpl->Update(timeStep); }

  static Array<const PropertyInfo> GetProperties()
  {
    return{
      EP_MAKE_PROPERTY(NearPlane, "Near depth plane", nullptr, 0),
      EP_MAKE_PROPERTY(FarPlane, "Far depth plane", nullptr, 0),
      EP_MAKE_PROPERTY_RO(FovY, "Field of view in the Y direction when in Perspective mode", nullptr, 0),
      EP_MAKE_PROPERTY_RO(OrthoHeight, "Height of the viewing frustrum in Orthographic mode", nullptr, 0),
      EP_MAKE_PROPERTY_RO(CameraMatrix, "Position of camera", nullptr, 0),
      EP_MAKE_PROPERTY_RO(ViewMatrix, "Position of camera", nullptr, 0),
    };
  }
  static Array<const MethodInfo> GetMethods()
  {
    return{
      EP_MAKE_METHOD(SetDepthPlanes, "Set the near and far depth planes:\n  setdepthplanes(near, far)"),
      EP_MAKE_METHOD(SetOrtho, "Set the projection mode to Orthographic with given ortho height"),
      EP_MAKE_METHOD(IsOrtho, "Returns true if the camera is in Orthographic mode"),
      EP_MAKE_METHOD(SetPerspective, "Set the projection mode to Perspective with given field of view"),
    };
  }
};

} // namespace ep

#endif
