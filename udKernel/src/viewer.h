#pragma once
#ifndef UDVIEWER_H
#define UDVIEWER_H

#include "udRender.h"
#include "udMath.h"

#include "hal/driver.h"
#include "components/component.h"

namespace ud
{
class View;
class Scene;
}

udResult udViewer_RunMainLoop(ud::View *pView, ud::Scene *pScene);
udResult udViewer_Quit();

#endif // UDVIEWER_H
