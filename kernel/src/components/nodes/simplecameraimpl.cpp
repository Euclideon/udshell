#include "ep/cpp/math.h"
#include "ep/cpp/platform.h"
#include "simplecameraimpl.h"
#include "kernel.h"

namespace ep {

static int mouseRemap[] = { -1, 0, 2, -1, 1 };

// ***************************************************************************************
// Author: Manu Evans, May 2015
SimpleCameraImpl::SimpleCameraImpl(Component *pInstance, Variant::VarMap initParams)
  : Super(pInstance)
{
  memset(keyState, 0, sizeof(keyState));
  const Variant *paramPos = initParams.Get("position");
  if (paramPos)
  {
    auto posArray = paramPos->as<Array<double, 3>>();
    if (posArray.length == 3)
    {
      Double3 position = Double3::create(posArray[0], posArray[1], posArray[2]);
      SetPosition(position);
    }
  }

  const Variant *paramOrientation = initParams.Get("orientation");
  if (paramOrientation)
  {
    auto oriArray = paramOrientation->as<Array<double, 3>>();
    if (oriArray.length == 3)
    {
      Double3 orientation = Double3::create(oriArray[0], oriArray[1], oriArray[2]);
      SetOrientation(orientation);
    }
  }

  const Variant *paramMat = initParams.Get("matrix");
  if (paramMat)
  {
    Array<double> matArray = paramMat->as<Array<double>>();
    if (matArray.length == 16)
      SetMatrix(Double4x4::create(matArray.ptr));
  }

  const Variant *paramSpeed = initParams.Get("speed");
  if (paramSpeed)
    SetSpeed(paramSpeed->asFloat());

  const Variant *paramInvert = initParams.Get("invertyaxis");
  if (paramInvert)
    SetInvertedYAxis(paramInvert->asBool());

  const Variant *paramHeli = initParams.Get("helicoptermode");
  if (paramHeli)
    SetHelicopterMode(paramHeli->asBool());
}

bool SimpleCameraImpl::ViewportInputEvent(const epInputEvent &ev)
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
bool SimpleCameraImpl::Update(double timeDelta)
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
  this->speed = Clamp(speed * tmpSpeed, 0.001, 999.0);

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
    ypr.y += EP_2PI;
  while (ypr.y >= EP_2PI)
    ypr.y -= EP_2PI;

  // clamp the pitch
  if (ypr.y > EP_HALF_PI && ypr.y < (EP_2PI - EP_HALF_PI))
    ypr.y = (ypr.y < EP_PI) ? EP_HALF_PI : (EP_2PI - EP_HALF_PI);

  // normalise yaw
  while (ypr.x < 0)
    ypr.x += EP_2PI;
  while (ypr.x >= EP_2PI)
    ypr.x -= EP_2PI;

  Double4x4 cam = pInstance->InstanceSuper::GetCameraMatrix();

  Double3 forward = cam.axis.y.toVector3();
  Double3 xAxis = cam.axis.x.toVector3();
  if(helicopterMode)
    forward = Cross3(Double3::create(0,0,1), xAxis);

  pos += forward*ty*tmpSpeed;
  pos += xAxis*tx*tmpSpeed;
  pos.z += tz*tmpSpeed;

  pInstance->InstanceSuper::SetMatrix(Double4x4::rotationYPR(ypr.x, ypr.y, ypr.z, pos));

  mouse.delta = {0 , 0};

  if (stateChanged)
  {
    pInstance->Changed.Signal(pos, ypr);
    stateChanged = false;
  }

  if (ty || tx || tz || pitch || yaw)
    return true;
  return false;
}

Variant SimpleCameraImpl::Save() const
{
  Variant var = pInstance->InstanceSuper::Save();
  Variant::VarMap params = var.asAssocArray();

  params.Insert("speed", speed);
  params.Insert("invertyaxis", (invertedYAxis == -1.0 ? true : false));
  params.Insert("helicoptermode", helicopterMode);

  return Variant(std::move(params));
}

} // namespace ep
