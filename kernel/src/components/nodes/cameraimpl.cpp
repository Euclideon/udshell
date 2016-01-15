#include "ep/cpp/platform.h"
#include "cameraimpl.h"
#include "kernel.h"

namespace ep
{

// ***************************************************************************************
// Author: Manu Evans, May 2015
CameraImpl::CameraImpl(Component *pInstance, Variant::VarMap initParams) : Super(pInstance)
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

  const Variant *paramNearPlane = initParams.Get("nearplane");
  if (paramNearPlane)
    SetNearPlane(paramNearPlane->asFloat());

  const Variant *paramFarPlane = initParams.Get("farplane");
  if (paramFarPlane)
    SetFarPlane(paramFarPlane->asFloat());
}

void CameraImpl::GetProjectionMatrix(double aspectRatio, Double4x4 *pMatrix) const
{
  if (!bOrtho)
    *pMatrix = Double4x4::perspective(fovY, aspectRatio, zNear, zFar);
  else
    *pMatrix = Double4x4::ortho(-orthoHeight*aspectRatio*0.5, orthoHeight*aspectRatio*0.5, -orthoHeight*0.5, orthoHeight*0.5, zNear, zFar);
}

Variant CameraImpl::Save() const
{
  Variant::VarMap params;

  params.Insert(KeyValuePair("matrix", pInstance->GetMatrix()));
  if (bOrtho)
    params.Insert(KeyValuePair("ortho", orthoHeight));
  else
    params.Insert(KeyValuePair("perspective", fovY));
  params.Insert(KeyValuePair("nearplane", zNear));
  params.Insert(KeyValuePair("nearplane", zFar));

  return Variant(std::move(params));
}

} // namespace ep
