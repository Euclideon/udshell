#include "udPlatform.h"
#include "udViewer.h"
#include "udCamera.h"
#include "udInput.h"

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udCamera::update()
{
  // update the camera
  double s;

  double pitch = 0.0, yaw = 0.0;
  double tx = 0.0, ty = 0.0, tz = 0.0;

  // rotations
  double rotSpeed = 3.14159*0.5*s_timeDelta;
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

  ty += udInput_State(udID_Keyboard, udKC_W)
        -udInput_State(udID_Keyboard, udKC_S)
        +udInput_State(udID_Keyboard, udKC_Up)
        -udInput_State(udID_Keyboard, udKC_Down);
  tx += udInput_State(udID_Keyboard, udKC_D)
        -udInput_State(udID_Keyboard, udKC_A)
        +udInput_State(udID_Keyboard, udKC_Right)
        -udInput_State(udID_Keyboard, udKC_Left);
  tz += udInput_State(udID_Keyboard, udKC_R)
        +udInput_State(udID_Keyboard, udKC_Home)
        -udInput_State(udID_Keyboard, udKC_F)
        -udInput_State(udID_Keyboard, udKC_End)
        +udInput_State(udID_Gamepad, udGC_ButtonDUp)
        -udInput_State(udID_Gamepad, udGC_ButtonDDown)
        +udInput_State(udID_Gamepad, udGC_ButtonRT)
        -udInput_State(udID_Gamepad, udGC_ButtonLT);

  // mode switch
  bool bToggle = !!udInput_WasPressed(udID_Gamepad, udGC_ButtonY);
  if(udInput_State(udID_Keyboard, udKC_1) || (bToggle && bHelicopter))
    helicopterMode(false);
  else if(udInput_State(udID_Keyboard, udKC_2) || (bToggle && !bHelicopter))
    helicopterMode(true);

  double speed = 1.0;
  if((s = -udInput_State(udID_Mouse, udMC_Wheel)) != 0.0)
    speed = pow(1.2, s);
  if(udInput_State(udID_Gamepad, udGC_ButtonA) || udInput_State(udID_Keyboard, udKC_Equals) || udInput_State(udID_Keyboard, udKC_NumpadPlus))
    speed = 1.0 + 1.0*s_timeDelta;
  if(udInput_State(udID_Gamepad, udGC_ButtonB) || udInput_State(udID_Keyboard, udKC_Hyphen) || udInput_State(udID_Keyboard, udKC_NumpadMinus))
    speed = 1.0 - 0.5*s_timeDelta;
  this->speed = udClamp(this->speed * speed, 0.001, 999.0);

  float multiplier = 1.f;
  if(udInput_State(udID_Keyboard, udKC_LShift) || udInput_State(udID_Keyboard, udKC_RShift) || udInput_State(udID_Gamepad, udGC_ButtonRB))
    multiplier *= 3.f;
  if(udInput_State(udID_Gamepad, udGC_ButtonLB))
    multiplier *= 0.333f;

  speed = this->speed * multiplier * s_timeDelta;

  // update the camera
  ypr.y += pitch;
  ypr.x += yaw;

  udDouble4x4 cam = getMatrix();

  udDouble3 forward = cam.axis.y.toVector3();
  udDouble3 xAxis = cam.axis.x.toVector3();
  if(bHelicopter)
    forward = udCross3(udDouble3::create(0,0,1), xAxis);

  pos += forward*ty*speed;
  pos += xAxis*tx*speed;
  pos.z += tz*speed;
}
