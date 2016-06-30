#pragma once
#ifndef EP_CAMERA_H
#define EP_CAMERA_H

#include "ep/cpp/component/node/node.h"
#include "ep/cpp/internal/i/icamera.h"
#include "ep/cpp/input.h"

namespace ep {

SHARED_CLASS(Camera);
SHARED_CLASS(View);

class Camera : public Node
{
  EP_DECLARE_COMPONENT_WITH_IMPL(ep, Camera, ICamera, Node, EPKERNEL_PLUGINVERSION, "Camera desc...", 0)

public:
  //! Gets the camera's world matrix.
  //! \return The camera's world matrix.
  //! \see GetViewMatrix, GetProjectionMatrix
  Double4x4 GetCameraMatrix() const { return pImpl->GetCameraMatrix(); }

  //! Gets the view matrix for this camera.
  //! \return The view matrix for the camera.
  //! \see GetCameraMatrix, GetProjectionMatrix
  Double4x4 GetViewMatrix() const { return pImpl->GetViewMatrix(); }

  //! Generates a projection matrix for this camera.
  //! \param aspectRatio Aspect ratio for the surface that the projection matrix will be used to render.
  //! \return The projection matrix for the camera with the specified aspect ratio.
  //! \see GetCameraMatrix, GetViewMatrix
  Double4x4 GetProjectionMatrix(double aspectRatio) const { return pImpl->GetProjectionMatrix(aspectRatio); }

  //! Sets the camera to have perspective projection.
  //! \param fovY Field of view on the Y axis.
  //! \return None.
  //! \see SetOrtho, GetProjectionMatrix
  void SetPerspective(double fovY) { pImpl->SetPerspective(fovY); }

  //! Sets the camera to have orthographic projection.
  //! \param orthoHeight The height of the orthographic projection volume.
  //! \return None.
  //! \remarks Note: The width of the projection will be implied by \a orthoHeight * \a aspectRatio when calling \c GetProjectionMatrix().
  //! \see SetPerspective, GetProjectionMatrix
  void SetOrtho(double orthoHeight) { pImpl->SetOrtho(orthoHeight); }

  double GetFovY() const { return pImpl->GetFovY(); }
  double GetOrthoHeight() const { return pImpl->GetOrthoHeight(); }
  bool IsOrtho() const { return pImpl->IsOrtho(); }

  void SetDepthPlanes(double zNear, double zFar) { pImpl->SetDepthPlanes(zNear, zFar); }
  void SetNearPlane(double zNear) { pImpl->SetNearPlane(zNear); }
  double GetNearPlane() const { return pImpl->GetNearPlane(); }
  void SetFarPlane(double zFar) { pImpl->SetFarPlane(zFar); }
  double GetFarPlane() const { return pImpl->GetFarPlane(); }

  Variant Save() const override { return pImpl->Save(); }

protected:
  EP_FRIENDS_WITH_IMPL(View);

  Camera(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Node(pType, pKernel, uid, initParams)
  {
    pImpl = CreateImpl(initParams);
  }

  virtual bool ViewportInputEvent(const ep::InputEvent &ev) { return pImpl->ViewportInputEvent(ev); }

  bool InputEvent(const ep::InputEvent &ev) override { return pImpl->InputEvent(ev); }
  bool Update(double timeStep) override { return pImpl->Update(timeStep); }

private:
  Array<const PropertyInfo> GetProperties() const;
  Array<const MethodInfo> GetMethods() const;
};

} // namespace ep

#endif
