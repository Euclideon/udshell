#include "udPlatform.h"

#include "camera.h"
#include "kernel.h"
#include "hal/input.h"

namespace ud
{

static CPropertyDesc props[] =
{
  {
    {
      "cameramatrix", // id
      "Position", // displayName
      "Position of camera", // description
    },
    &Camera::GetCameraMatrix,
    nullptr
  },
  {
    {
      "viewmatrix", // id
      "Position", // displayName
      "Position of camera", // description
    },
    &Camera::GetViewMatrix,
    nullptr
  }
};
static CMethodDesc methods[] =
{
  {
    {
      "setdepthplanes",
      "Set the near and far depth planes:\n  setdepthplanes(near, far)",
    },
    &Camera::SetDepthPlanes // method
  }
};
ComponentDesc Camera::descriptor =
{
  &Node::descriptor, // pSuperDesc

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "camera",      // id
  "Camera",    // displayName
  "Is a camera", // description

  udSlice<CPropertyDesc>(props, UDARRAYSIZE(props)), // propeties
  udSlice<CMethodDesc>(methods, UDARRAYSIZE(methods)) // methods
};


static CPropertyDesc simpleCameraProps[] =
{
  {
    {
      "matrix", // id
      "Matrix", // displayName
      "Local matrix", // description
    },
    &SimpleCamera::GetMatrix, // getter
    &SimpleCamera::SetMatrix  // setter
  },
  {
    {
      "position", // id
      "Position", // displayName
      "Local position", // description
    },
    &SimpleCamera::GetPosition, // getter
    &SimpleCamera::SetPosition  // setter
  },
  {
    {
      "orientation", // id
      "Orientation", // displayName
      "Camera orientation (YPR)", // description
    },
    nullptr, // getter
    &SimpleCamera::SetOrientation  // setter
  },
  {
    {
      "speed", // id
      "Speed", // displayName
      "Camera speed", // description
    },
    nullptr, // getter
    &SimpleCamera::SetSpeed  // setter
  }
};
ComponentDesc SimpleCamera::descriptor =
{
  &Camera::descriptor, // pSuperDesc

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "simplecamera",       // id
  "SimpleCamera",     // displayName
  "Is a simple camera", // description

  udSlice<CPropertyDesc>(simpleCameraProps, UDARRAYSIZE(simpleCameraProps)) // propeties
};


// ***************************************************************************************
// Author: Manu Evans, May 2015
void Camera::GetProjectionMatrix(double aspectRatio, udDouble4x4 *pMatrix) const
{
  if (!bOrtho)
    *pMatrix = udDouble4x4::perspective(fovY, aspectRatio, zNear, zFar);
  else
    *pMatrix = udDouble4x4::ortho(-orthoHeight*aspectRatio*0.5, orthoHeight*aspectRatio*0.5, -orthoHeight*0.5, orthoHeight*0.5, zNear, zFar);
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
bool SimpleCamera::ViewportInputEvent(const udInputEvent &ev)
{
  if (ev.deviceType == udID_Keyboard)
  {
    if (ev.eventType == udInputEvent::EventType::Key)
    {
      switch (ev.key.key)
      {
        case udKC_W:
        case udKC_Up:
          keyState[(int)Keys::Up] += ev.key.state ? 1 : -1;
          return true;
        case udKC_S:
        case udKC_Down:
          keyState[(int)Keys::Down] += ev.key.state ? 1 : -1;
          return true;
        case udKC_A:
        case udKC_Left:
          keyState[(int)Keys::Left] += ev.key.state ? 1 : -1;
          return true;
        case udKC_D:
        case udKC_Right:
          keyState[(int)Keys::Right] += ev.key.state ? 1 : -1;
          return true;
        case udKC_R:
        case udKC_PageUp:
          keyState[(int)Keys::Elevate] += ev.key.state ? 1 : -1;
          return true;
        case udKC_F:
        case udKC_PageDown:
          keyState[(int)Keys::Descend] += ev.key.state ? 1 : -1;
          return true;
        case udKC_LShift:
        case udKC_RShift:
          keyState[(int)Keys::Boost] += ev.key.state ? 1 : -1;
          return true;
      }
    }
  }
  else if (ev.deviceType == udID_Mouse)
  {

  }

  return false;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
bool SimpleCamera::Update(double timeDelta)
{
  // update the camera
  double s;

  double pitch = 0.0, yaw = 0.0;
  double tx = 0.0, ty = 0.0, tz = 0.0;

  // rotations
  double rotSpeed = 3.14159*0.5*timeDelta;
  if(fabs(s = udInput_State(udID_Gamepad, udGC_AxisRY)) > 0.2f)
    pitch += s*yInvert*rotSpeed;
  if(fabs(s = udInput_State(udID_Gamepad, udGC_AxisRX)) > 0.2f)
    yaw += -s*rotSpeed;

  if(udInput_State(udID_Mouse, udMC_LeftButton))
  {
    pitch += udInput_State(udID_Mouse, udMC_YDelta)*yInvert*0.005;
    yaw += -udInput_State(udID_Mouse, udMC_XDelta)*0.005;
  }

  yaw += udInput_State(udID_Keyboard, udKC_Q)*rotSpeed
         -udInput_State(udID_Keyboard, udKC_E)*rotSpeed;

  // translations
  if(fabs(s = udInput_State(udID_Gamepad, udGC_AxisLY)) > 0.2f)
    ty += -s;
  if(fabs(s = udInput_State(udID_Gamepad, udGC_AxisLX)) > 0.2f)
    tx += s;

  ty += keyState[(int)Keys::Up]
        -keyState[(int)Keys::Down];
  tx += keyState[(int)Keys::Right]
        -keyState[(int)Keys::Left];
  tz += keyState[(int)Keys::Elevate]
        -keyState[(int)Keys::Descend]
        +udInput_State(udID_Gamepad, udGC_ButtonDUp)
        -udInput_State(udID_Gamepad, udGC_ButtonDDown)
        +udInput_State(udID_Gamepad, udGC_ButtonRT)
        -udInput_State(udID_Gamepad, udGC_ButtonLT);

  // mode switch
#if 0
  bool bToggle = !!udInput_WasPressed(udID_Gamepad, udGC_ButtonY);
  if(udInput_State(udID_Keyboard, udKC_1) || (bToggle && bHelicopter))
    helicopterMode(false);
  else if(udInput_State(udID_Keyboard, udKC_2) || (bToggle && !bHelicopter))
    helicopterMode(true);
#endif // 0
  double speed = 1.0;
  if((s = -udInput_State(udID_Mouse, udMC_Wheel)) != 0.0)
    speed = pow(1.2, s);
  if(udInput_State(udID_Gamepad, udGC_ButtonA) || udInput_State(udID_Keyboard, udKC_Equals) || udInput_State(udID_Keyboard, udKC_NumpadPlus))
    speed = 1.0 + 1.0*timeDelta;
  if(udInput_State(udID_Gamepad, udGC_ButtonB) || udInput_State(udID_Keyboard, udKC_Hyphen) || udInput_State(udID_Keyboard, udKC_NumpadMinus))
    speed = 1.0 - 0.5*timeDelta;
  this->speed = udClamp(this->speed * speed, 0.001, 999.0);

  float multiplier = 1.f;
  if (keyState[(int)Keys::Boost] || udInput_State(udID_Gamepad, udGC_ButtonRB))
    multiplier *= 3.f;
  if(udInput_State(udID_Gamepad, udGC_ButtonLB))
    multiplier *= 0.333f;

  speed = this->speed * multiplier * timeDelta;

  // update the camera
  ypr.y += pitch;
  ypr.x += yaw;

  // normalise pitch
  while (ypr.y < 0)
    ypr.y += UD_2PI;
  while (ypr.y >= UD_2PI)
    ypr.y -= UD_2PI;

  // clamp the pitch
  if (ypr.y > UD_HALF_PI && ypr.y < (UD_2PI - UD_HALF_PI))
    ypr.y = (ypr.y < UD_PI) ? UD_HALF_PI : (UD_2PI - UD_HALF_PI);

  // normalise yaw
  while (ypr.x < 0)
    ypr.x += UD_2PI;
  while (ypr.x >= UD_2PI)
    ypr.x -= UD_2PI;

  udDouble4x4 cam = GetCameraMatrix();

  udDouble3 forward = cam.axis.y.toVector3();
  udDouble3 xAxis = cam.axis.x.toVector3();
  if(bHelicopter)
    forward = udCross3(udDouble3::create(0,0,1), xAxis);

  pos += forward*ty*speed;
  pos += xAxis*tx*speed;
  pos.z += tz*speed;

  matrix = udDouble4x4::rotationYPR(ypr.x, ypr.y, ypr.z, pos);

  if (ty || tx || tz || pitch || yaw)
    return true;
  return false;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
Component *SimpleCamera::CreateInstance(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, udInitParams initParams)
{
  return udNew(SimpleCamera, pType, pKernel, uid, initParams);
}

} // namespace ud
