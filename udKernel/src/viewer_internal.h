#pragma once
#ifndef UDVIEWER_INTERNAL_H
#define UDVIEWER_INTERNAL_H

#include "viewer.h"
#include "udRender.h"


void udViewer_MainLoop();
void udViewer_BeginFrame();
void udViewer_EndFrame();
void udViewer_Update();
void udViewer_Draw();

void udViewer_ResizeFrame(int width, int height);
/*
udViewerInstance* udViewerDriver_CreateInstance();
void udViewerDriver_Init(udViewerInstance *pInstance);
void udViewerDriver_Deinit(udViewerInstance *pInstance);
void udViewerDriver_RunMainLoop(udViewerInstance *pInstance);
void udViewerDriver_Quit(udViewerInstance *pInstance);
*/
void udGPU_Init();

#endif
