#include "ep/cpp/platform.h"
#include "camera.h"
#include "kernel.h"

namespace ep
{

// ***************************************************************************************
// Author: Manu Evans, May 2015
void Camera::GetProjectionMatrix(double aspectRatio, Double4x4 *pMatrix) const
{
  if (!bOrtho)
    *pMatrix = Double4x4::perspective(fovY, aspectRatio, zNear, zFar);
  else
    *pMatrix = Double4x4::ortho(-orthoHeight*aspectRatio*0.5, orthoHeight*aspectRatio*0.5, -orthoHeight*0.5, orthoHeight*0.5, zNear, zFar);
}

Variant Camera::Save() const
{
  Variant::VarMap params;

  params.Insert(KeyValuePair("matrix", matrix));
  if (bOrtho)
    params.Insert(KeyValuePair("ortho", orthoHeight));
  else
    params.Insert(KeyValuePair("perspective", fovY));
  params.Insert(KeyValuePair("depthplanes", Array<double, 2>({ zNear, zFar })));

  return Variant(std::move(params));
}

static int mouseRemap[] = { -1, 0, 2, -1, 1 };

// ***************************************************************************************
// Author: Manu Evans, May 2015
bool SimpleCamera::ViewportInputEvent(const epInputEvent &ev)
{
  if (ev.deviceType == epID_Keyboard)
  {
    if (ev.eventType == epInputEvent::EventType::Key)
    {
      switch (ev.key.key)
      {
        case epKC_W:
        case epKC_Up:
          keyState[(int)Keys::Up] += ev.key.state ? 1 : -1;
          stateChanged = true;
          break;
        case epKC_S:
        case epKC_Down:
          keyState[(int)Keys::Down] += ev.key.state ? 1 : -1;
          stateChanged = true;
          break;
        case epKC_A:
        case epKC_Left:
          keyState[(int)Keys::Left] += ev.key.state ? 1 : -1;
          stateChanged = true;
          break;
        case epKC_D:
        case epKC_Right:
          keyState[(int)Keys::Right] += ev.key.state ? 1 : -1;
          stateChanged = true;
          break;
        case epKC_R:
        case epKC_PageUp:
          keyState[(int)Keys::Elevate] += ev.key.state ? 1 : -1;
          stateChanged = true;
          break;
        case epKC_F:
        case epKC_PageDown:
          keyState[(int)Keys::Descend] += ev.key.state ? 1 : -1;
          stateChanged = true;
          break;
        case epKC_LShift:
        case epKC_RShift:
          keyState[(int)Keys::Boost] += ev.key.state ? 1 : -1;
          stateChanged = true;
          break;
        case epKC_1:
          keyState[(int)Keys::Key_1] += ev.key.state ? 1 : -1;
          stateChanged = true;
          break;
        case epKC_2:
          keyState[(int)Keys::Key_2] += ev.key.state ? 1 : -1;
          stateChanged = true;
          break;
      }
    }
  }
  else if (ev.deviceType == epID_Mouse)
  {
    if (ev.eventType == epInputEvent::Move)
    {
      if (mouse.buttons[Mouse::Left])
      {
        if (mouse.stateChanged[Mouse::Left])
        {
          mouse.absolute.x = ev.move.xAbsolute;
          mouse.absolute.y = ev.move.yAbsolute;
          mouse.prevAbsolute = mouse.absolute;
          mouse.stateChanged[Mouse::Left] = 0;
        }
      }

      mouse.prevAbsolute = mouse.absolute;
      mouse.absolute.x = ev.move.xAbsolute;
      mouse.absolute.y = ev.move.yAbsolute;

      mouse.delta = mouse.prevAbsolute - mouse.absolute;
      stateChanged = true;
    }
    else if (ev.eventType == epInputEvent::Key)
    {
      int button = mouseRemap[ev.key.key];

      if (!mouse.buttons[button])
        mouse.stateChanged[button] = 1;

      mouse.buttons[button] = (char)ev.key.state;
      stateChanged = true;
    }
  }

  return stateChanged;
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
  if(fabs(s = epInput_State(epID_Gamepad, epGC_AxisRY)) > 0.2f)
    pitch += s*invertedYAxis*rotSpeed;
  if(fabs(s = epInput_State(epID_Gamepad, epGC_AxisRX)) > 0.2f)
    yaw += -s*rotSpeed;

  if (mouse.buttons[Mouse::Left])
  {
    pitch += mouse.delta.y * invertedYAxis * 0.005;
    yaw += mouse.delta.x*0.005;
  }

  if(epInput_State(epID_Mouse, epMC_LeftButton))
  {
    pitch += epInput_State(epID_Mouse, epMC_YDelta)*invertedYAxis*0.005;
    yaw += -epInput_State(epID_Mouse, epMC_XDelta)*0.005;
  }

  yaw += epInput_State(epID_Keyboard, epKC_Q)*rotSpeed
         -epInput_State(epID_Keyboard, epKC_E)*rotSpeed;

  // translations
  if(fabs(s = epInput_State(epID_Gamepad, epGC_AxisLY)) > 0.2f)
    ty += -s;
  if(fabs(s = epInput_State(epID_Gamepad, epGC_AxisLX)) > 0.2f)
    tx += s;

  ty += keyState[(int)Keys::Up]
        -keyState[(int)Keys::Down];
  tx += keyState[(int)Keys::Right]
        -keyState[(int)Keys::Left];
  tz += keyState[(int)Keys::Elevate]
        -keyState[(int)Keys::Descend]
        +epInput_State(epID_Gamepad, epGC_ButtonDUp)
        -epInput_State(epID_Gamepad, epGC_ButtonDDown)
        +epInput_State(epID_Gamepad, epGC_ButtonRT)
        -epInput_State(epID_Gamepad, epGC_ButtonLT);

  ty += mouse.buttons[Mouse::Right];

  if (keyState[(int)Keys::Key_1])
    SetHelicopterMode(false);
  else if (keyState[(int)Keys::Key_2])
    SetHelicopterMode(true);

  // mode switch
#if 0
  bool bToggle = !!epInput_WasPressed(epID_Gamepad, epGC_ButtonY);
  if(epInput_State(epID_Keyboard, epKC_1) || (bToggle && helicopterMode))
    helicopterMode(false);
  else if(epInput_State(epID_Keyboard, epKC_2) || (bToggle && !helicopterMode))
    helicopterMode(true);
#endif // 0
  double tmpSpeed = 1.0;
  if((s = -epInput_State(epID_Mouse, epMC_Wheel)) != 0.0)
    tmpSpeed = pow(1.2, s);
  if(epInput_State(epID_Gamepad, epGC_ButtonA) || epInput_State(epID_Keyboard, epKC_Equals) || epInput_State(epID_Keyboard, epKC_NumpadPlus))
    tmpSpeed = 1.0 + 1.0*timeDelta;
  if(epInput_State(epID_Gamepad, epGC_ButtonB) || epInput_State(epID_Keyboard, epKC_Hyphen) || epInput_State(epID_Keyboard, epKC_NumpadMinus))
    tmpSpeed = 1.0 - 0.5*timeDelta;
  this->speed = udClamp(speed * tmpSpeed, 0.001, 999.0);

  float multiplier = 1.f;
  if (keyState[(int)Keys::Boost] || epInput_State(epID_Gamepad, epGC_ButtonRB))
    multiplier *= 3.f;
  if(epInput_State(epID_Gamepad, epGC_ButtonLB))
    multiplier *= 0.333f;

  tmpSpeed = speed * multiplier * timeDelta;

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

  Double4x4 cam = GetCameraMatrix();

  Double3 forward = cam.axis.y.toVector3();
  Double3 xAxis = cam.axis.x.toVector3();
  if(helicopterMode)
    forward = Cross3(Double3::create(0,0,1), xAxis);

  pos += forward*ty*tmpSpeed;
  pos += xAxis*tx*tmpSpeed;
  pos.z += tz*tmpSpeed;

  matrix = Double4x4::rotationYPR(ypr.x, ypr.y, ypr.z, pos);

  mouse.delta = {0 , 0};

  if (stateChanged)
  {
    Changed.Signal(pos, ypr);
    stateChanged = false;
  }

  if (ty || tx || tz || pitch || yaw)
    return true;
  return false;
}

Variant SimpleCamera::Save() const
{
  Variant var = Camera::Save();
  Variant::VarMap params = var.asAssocArray();

  params.Insert("speed", speed);
  params.Insert("invertyaxis", (invertedYAxis == -1.0 ? true : false));
  params.Insert("helicoptermode", helicopterMode);

  return Variant(std::move(params));
}

} // namespace ep
