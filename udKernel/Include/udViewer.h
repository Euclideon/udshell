#pragma once
#ifndef UDVIEWER_H
#define UDVIEWER_H

#include "udDriver.h"
#include "udRender.h"
#include "udMath.h"
#include "udComponent.h"

namespace udKernel
{
class View;
class Scene;
}

udResult udViewer_RunMainLoop(udKernel::View *pView, udKernel::Scene *pScene);
udResult udViewer_Quit();

#endif // UDVIEWER_H
