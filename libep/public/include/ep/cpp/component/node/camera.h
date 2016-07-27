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
  Double4x4 getCameraMatrix() const { return pImpl->GetCameraMatrix(); }

  //! Gets the view matrix for this camera.
  //! \return The view matrix for the camera.
  //! \see GetCameraMatrix, GetProjectionMatrix
  Double4x4 getViewMatrix() const { return pImpl->GetViewMatrix(); }

  //! Generates a projection matrix for this camera.
  //! \param aspectRatio Aspect ratio for the surface that the projection matrix will be used to render.
  //! \return The projection matrix for the camera with the specified aspect ratio.
  //! \see GetCameraMatrix, GetViewMatrix
  Double4x4 getProjectionMatrix(double aspectRatio) const { return pImpl->GetProjectionMatrix(aspectRatio); }

  //! Sets the camera to have perspective projection.
  //! \param fovY Field of view on the Y axis.
  //! \return None.
  //! \see SetOrtho, GetProjectionMatrix
  void setPerspective(double fovY) { pImpl->SetPerspective(fovY); }

  //! Sets the camera to have orthographic projection.
  //! \param orthoHeight The height of the orthographic projection volume.
  //! \return None.
  //! \remarks Note: The width of the projection will be implied by \a orthoHeight * \a aspectRatio when calling \c GetProjectionMatrix().
  //! \see SetPerspective, GetProjectionMatrix
  void setOrtho(double orthoHeight) { pImpl->SetOrtho(orthoHeight); }

  double getFovY() const { return pImpl->GetFovY(); }
  double getOrthoHeight() const { return pImpl->GetOrthoHeight(); }
  bool isOrtho() const { return pImpl->IsOrtho(); }

  void setDepthPlanes(double zNear, double zFar) { pImpl->SetDepthPlanes(zNear, zFar); }
  void setNearPlane(double zNear) { pImpl->SetNearPlane(zNear); }
  double getNearPlane() const { return pImpl->GetNearPlane(); }
  void setFarPlane(double zFar) { pImpl->SetFarPlane(zFar); }
  double getFarPlane() const { return pImpl->GetFarPlane(); }

  Variant save() const override { return pImpl->Save(); }

protected:
  EP_FRIENDS_WITH_IMPL(View);

  Camera(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Node(pType, pKernel, uid, initParams)
  {
    pImpl = createImpl(initParams);
  }

  virtual bool ViewportInputEvent(const ep::InputEvent &ev) { return pImpl->ViewportInputEvent(ev); }

  bool inputEvent(const ep::InputEvent &ev) override { return pImpl->InputEvent(ev); }
  bool update(double timeStep) override { return pImpl->Update(timeStep); }

private:
  Array<const PropertyInfo> getProperties() const;
  Array<const MethodInfo> getMethods() const;
};

} // namespace ep

#endif
