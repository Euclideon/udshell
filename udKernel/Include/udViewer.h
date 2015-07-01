#pragma once
#ifndef UDVIEWER_H
#define UDVIEWER_H

#include "udDriver.h"
#include "udRender.h"
#include "udMath.h"
#include "udComponent.h"


udResult udViewer_RunMainLoop(class udView *pView, class udScene *pScene);
udResult udViewer_Quit();

#endif // UDVIEWER_H
