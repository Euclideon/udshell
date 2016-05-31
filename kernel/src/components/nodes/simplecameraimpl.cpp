#include "ep/cpp/math.h"
#include "ep/cpp/platform.h"
#include "simplecameraimpl.h"
#include "ep/cpp/kernel.h"

namespace ep {

static int mouseRemap[] = { -1, 0, 2, -1, 1 };

Array<const PropertyInfo> SimpleCamera::GetProperties() const
{
  return{
    EP_MAKE_PROPERTY_WO(Matrix, "Local matrix", nullptr, 0),
    EP_MAKE_PROPERTY_WO(Position, "Local position", nullptr, 0),
    EP_MAKE_PROPERTY_WO(Orientation, "Camera orientation (YPR)", nullptr, 0),
    EP_MAKE_PROPERTY_WO(Speed, "Camera speed", nullptr, 0),
    EP_MAKE_PROPERTY(HelicopterMode, "Helicopter Mode", nullptr, 0),
    EP_MAKE_PROPERTY(InvertedYAxis, "InvertYAxis", nullptr, 0),
  };
}
Array<const EventInfo> SimpleCamera::GetEvents() const
{
  return{
    EP_MAKE_EVENT(Changed, "The camera changed")
  };
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
SimpleCameraImpl::SimpleCameraImpl(Component *pInstance, Variant::VarMap initParams)
  : ImplSuper(pInstance)
{
  memset(keyState, 0, sizeof(keyState));
  const Variant *paramPos = initParams.get("position");
  if (paramPos)
  {
    auto posArray = paramPos->as<Array<double, 3>>();
    if (posArray.length == 3)
    {
      Double3 position = Double3::create(posArray[0], posArray[1], posArray[2]);
      SetPosition(position);
    }
  }

  const Variant *paramOrientation = initParams.get("orientation");
  if (paramOrientation)
  {
    auto oriArray = paramOrientation->as<Array<double, 3>>();
    if (oriArray.length == 3)
    {
      Double3 orientation = Double3::create(oriArray[0], oriArray[1], oriArray[2]);
      SetOrientation(orientation);
    }
  }

  const Variant *paramMat = initParams.get("matrix");
  if (paramMat)
  {
    Array<double> matArray = paramMat->as<Array<double>>();
    if (matArray.length == 16)
      SetMatrix(Double4x4::create(matArray.ptr));
  }

  const Variant *paramSpeed = initParams.get("speed");
  if (paramSpeed)
    SetSpeed(paramSpeed->asFloat());

  const Variant *paramInvert = initParams.get("invertyaxis");
  if (paramInvert)
    SetInvertedYAxis(paramInvert->asBool());

  const Variant *paramHeli = initParams.get("helicoptermode");
  if (paramHeli)
    SetHelicopterMode(paramHeli->asBool());
}

bool SimpleCameraImpl::ViewportInputEvent(const InputEvent &ev)
{
  if (ev.eventType == InputEvent::EventType::Focus)
  {
    if (!ev.hasFocus)
    {
      memset(keyState, 0, sizeof(keyState));
      stateChanged = true;
    }
  }
  else if (ev.deviceType == InputDevice::Keyboard)
  {
    if (ev.eventType == InputEvent::EventType::Key)
    {
      switch ((KeyCode)ev.key.key)
      {
        case KeyCode::W:
        case KeyCode::Up:
          keyState[(int)Keys::Up] = ev.key.state ? 1 : 0;
          stateChanged = true;
          break;
        case KeyCode::S:
        case KeyCode::Down:
          keyState[(int)Keys::Down] = ev.key.state ? 1 : 0;
          stateChanged = true;
          break;
        case KeyCode::A:
        case KeyCode::Left:
          keyState[(int)Keys::Left] = ev.key.state ? 1 : 0;
          stateChanged = true;
          break;
        case KeyCode::D:
        case KeyCode::Right:
          keyState[(int)Keys::Right] = ev.key.state ? 1 : 0;
          stateChanged = true;
          break;
        case KeyCode::R:
        case KeyCode::PageUp:
          keyState[(int)Keys::Elevate] = ev.key.state ? 1 : 0;
          stateChanged = true;
          break;
        case KeyCode::F:
        case KeyCode::PageDown:
          keyState[(int)Keys::Descend] = ev.key.state ? 1 : 0;
          stateChanged = true;
          break;
        case KeyCode::LShift:
        case KeyCode::RShift:
          keyState[(int)Keys::Boost] = ev.key.state ? 1 : 0;
          stateChanged = true;
          break;
        case KeyCode::_1:
          keyState[(int)Keys::Key_1] = ev.key.state ? 1 : 0;
          stateChanged = true;
          break;
        case KeyCode::_2:
          keyState[(int)Keys::Key_2] = ev.key.state ? 1 : 0;
          stateChanged = true;
          break;
        default:
          break;
      }
    }
  }
  else if (ev.deviceType == InputDevice::Mouse)
  {
    if (ev.eventType == InputEvent::EventType::Move)
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
    else if (ev.eventType == InputEvent::EventType::Key)
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
  if(fabs(s = epInput_State(InputDevice::Gamepad, GamepadControl::AxisRY)) > 0.2f)
    pitch += s*invertedYAxis*rotSpeed;
  if(fabs(s = epInput_State(InputDevice::Gamepad, GamepadControl::AxisRX)) > 0.2f)
    yaw += -s*rotSpeed;

  if (mouse.buttons[Mouse::Left])
  {
    pitch += mouse.delta.y * invertedYAxis * 0.005;
    yaw += mouse.delta.x*0.005;
  }

  if(epInput_State(InputDevice::Mouse, MouseControls::LeftButton))
  {
    pitch += epInput_State(InputDevice::Mouse, MouseControls::YDelta)*invertedYAxis*0.005;
    yaw += -epInput_State(InputDevice::Mouse, MouseControls::XDelta)*0.005;
  }

  yaw += epInput_State(InputDevice::Keyboard, (int)KeyCode::Q)*rotSpeed
         -epInput_State(InputDevice::Keyboard, (int)KeyCode::E)*rotSpeed;

  // translations
  if(fabs(s = epInput_State(InputDevice::Gamepad, GamepadControl::AxisLY)) > 0.2f)
    ty += -s;
  if(fabs(s = epInput_State(InputDevice::Gamepad, GamepadControl::AxisLX)) > 0.2f)
    tx += s;

  ty += keyState[(int)Keys::Up]
        -keyState[(int)Keys::Down];
  tx += keyState[(int)Keys::Right]
        -keyState[(int)Keys::Left];
  tz += keyState[(int)Keys::Elevate]
        -keyState[(int)Keys::Descend]
        +epInput_State(InputDevice::Gamepad, GamepadControl::ButtonDUp)
        -epInput_State(InputDevice::Gamepad, GamepadControl::ButtonDDown)
        +epInput_State(InputDevice::Gamepad, GamepadControl::ButtonRT)
        -epInput_State(InputDevice::Gamepad, GamepadControl::ButtonLT);

  ty += mouse.buttons[Mouse::Right];

  if (keyState[(int)Keys::Key_1])
    SetHelicopterMode(false);
  else if (keyState[(int)Keys::Key_2])
    SetHelicopterMode(true);

  // mode switch
#if 0
  bool bToggle = !!epInput_WasPressed(InputDevice::Gamepad, GamepadControl::ButtonY);
  if(epInput_State(InputDevice::Keyboard, KeyCode::1) || (bToggle && helicopterMode))
    helicopterMode(false);
  else if(epInput_State(InputDevice::Keyboard, KeyCode::2) || (bToggle && !helicopterMode))
    helicopterMode(true);
#endif // 0
  double tmpSpeed = 1.0;
  if((s = -epInput_State(InputDevice::Mouse, MouseControls::Wheel)) != 0.0)
    tmpSpeed = pow(1.2, s);
  if(epInput_State(InputDevice::Gamepad, (int)GamepadControl::ButtonA) || epInput_State(InputDevice::Keyboard, (int)KeyCode::Equals) || epInput_State(InputDevice::Keyboard, (int)KeyCode::NumpadPlus))
    tmpSpeed = 1.0 + 1.0*timeDelta;
  if(epInput_State(InputDevice::Gamepad, (int)GamepadControl::ButtonB) || epInput_State(InputDevice::Keyboard, (int)KeyCode::Hyphen) || epInput_State(InputDevice::Keyboard, (int)KeyCode::NumpadMinus))
    tmpSpeed = 1.0 - 0.5*timeDelta;
  this->speed = Clamp(speed * tmpSpeed, 0.001, 999.0);

  float multiplier = 1.f;
  if (keyState[(int)Keys::Boost] || epInput_State(InputDevice::Gamepad, GamepadControl::ButtonRB))
    multiplier *= 3.f;
  if(epInput_State(InputDevice::Gamepad, GamepadControl::ButtonLB))
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

  Double4x4 cam = pInstance->Super::GetCameraMatrix();

  Double3 forward = cam.axis.y.toVector3();
  Double3 xAxis = cam.axis.x.toVector3();
  if(helicopterMode)
    forward = Cross3(Double3::create(0,0,1), xAxis);

  pos += forward*ty*tmpSpeed;
  pos += xAxis*tx*tmpSpeed;
  pos.z += tz*tmpSpeed;

  pInstance->Super::SetMatrix(Double4x4::rotationYPR(ypr.x, ypr.y, ypr.z, pos));

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
  Variant var = pInstance->Super::Save();
  Variant::VarMap params = var.asAssocArray();

  params.insert("speed", speed);
  params.insert("invertyaxis", (invertedYAxis == -1.0 ? true : false));
  params.insert("helicoptermode", helicopterMode);

  return Variant(std::move(params));
}

} // namespace ep
