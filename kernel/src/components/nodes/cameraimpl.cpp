#include "ep/cpp/platform.h"
#include "cameraimpl.h"
#include "ep/cpp/kernel.h"

namespace ep {

Array<const PropertyInfo> Camera::GetProperties() const
{
  return{
    EP_MAKE_PROPERTY(NearPlane, "Near depth plane", nullptr, 0),
    EP_MAKE_PROPERTY(FarPlane, "Far depth plane", nullptr, 0),
    EP_MAKE_PROPERTY_RO(FovY, "Field of view in the Y direction when in Perspective mode", nullptr, 0),
    EP_MAKE_PROPERTY_RO(OrthoHeight, "Height of the viewing frustrum in Orthographic mode", nullptr, 0),
    EP_MAKE_PROPERTY_RO(CameraMatrix, "Position of camera", nullptr, 0),
    EP_MAKE_PROPERTY_RO(ViewMatrix, "Position of camera", nullptr, 0),
    EP_MAKE_PROPERTY_EXPLICIT("IsOrtho", "If the camera is in Orthographic mode", EP_MAKE_GETTER(IsOrtho), nullptr, nullptr, 0),
  };
}
Array<const MethodInfo> Camera::GetMethods() const
{
  return{
    EP_MAKE_METHOD(SetDepthPlanes, "Set the near and far depth planes:\n  setdepthplanes(near, far)"),
    EP_MAKE_METHOD(SetOrtho, "Set the projection mode to Orthographic with given ortho height"),
    EP_MAKE_METHOD(SetPerspective, "Set the projection mode to Perspective with given field of view"),
    EP_MAKE_METHOD(GetProjectionMatrix, "Generate a projection matrix for the camera: getprojectionmatrix(aspectratio)"),
  };
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
CameraImpl::CameraImpl(Component *pInstance, Variant::VarMap initParams)
  : ImplSuper(pInstance)
{
  const Variant *perspParam = initParams.get("perspective");
  if (perspParam)
    SetPerspective(perspParam->asFloat());

  const Variant *orthoParam = initParams.get("ortho");
  if (orthoParam)
    SetOrtho(orthoParam->asFloat());

  const Variant *paramDepthPlanes = initParams.get("depthplanes");
  if (paramDepthPlanes)
  {
    auto depthPlanesArray = paramDepthPlanes->as<Array<double, 2>>();
    if (depthPlanesArray.length == 2)
      SetDepthPlanes(depthPlanesArray[0], depthPlanesArray[1]);
  }

  const Variant *paramNearPlane = initParams.get("nearplane");
  if (paramNearPlane)
    SetNearPlane(paramNearPlane->asFloat());

  const Variant *paramFarPlane = initParams.get("farplane");
  if (paramFarPlane)
    SetFarPlane(paramFarPlane->asFloat());
}

Double4x4 CameraImpl::GetProjectionMatrix(double aspectRatio) const
{
  if (!bOrtho)
    return Double4x4::perspective(fovY, aspectRatio, zNear, zFar);
  else
    return Double4x4::ortho(-orthoHeight*aspectRatio*0.5, orthoHeight*aspectRatio*0.5, -orthoHeight*0.5, orthoHeight*0.5, zNear, zFar);
}

Variant CameraImpl::Save() const
{
  Variant::VarMap params;

  params.insert(KeyValuePair("matrix", pInstance->GetMatrix()));
  if (bOrtho)
    params.insert(KeyValuePair("ortho", orthoHeight));
  else
    params.insert(KeyValuePair("perspective", fovY));
  params.insert(KeyValuePair("nearplane", zNear));
  params.insert(KeyValuePair("farplane", zFar));

  return Variant(std::move(params));
}

} // namespace ep
