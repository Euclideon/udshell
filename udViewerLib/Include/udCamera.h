#pragma once
#ifndef UDCAMERA_H
#define UDCAMERA_H

#include "udMath.h"

struct udCamera
{
  udDouble3 pos;
  udDouble3 ypr;

  double yInvert;
  double speed;

  bool bHelicopter;

  udCamera() : pos(udDouble3::zero()), ypr(udDouble3::zero()), yInvert(-1.0), speed(1.0), bHelicopter(false) {}

  udDouble4x4 getMatrix() { return udDouble4x4::rotationYPR(ypr.x, ypr.y, ypr.z, pos); }

  void setPosition(udDouble3 _pos) { pos = _pos; }
  void setOrientation(udDouble3 _ypr) { ypr = _ypr; }
  void setFromMatrix(const udDouble4x4 &matrix) { pos = matrix.axis.t.toVector3(); ypr = matrix.extractYPR(); }

  void invertYAxis(bool bInvert) { yInvert = bInvert ? -1.0 : 1.0; }
  void helicopterMode(bool bEnable) { bHelicopter = bEnable; }

  void update();
};

#endif
