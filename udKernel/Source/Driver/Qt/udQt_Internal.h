#pragma once
#ifndef UDQT_INTERNAL_H
#define UDQT_INTERNAL_H

#include "udDriver.h"

#include <QOpenGLFunctions_2_0>
#include <QOpenGLDebugLogger>

// internal storage of the Qt GL Context
struct udQtGLContext
{
  QOpenGLFunctions_2_0 *pFunc;
  QOpenGLDebugLogger *pDebugger;
};

extern udQtGLContext s_QtGLContext;

#endif
