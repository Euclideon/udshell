#pragma once
#ifndef UDVIEWERLIB_H
#define UDVIEWERLIB_H

#include "udViewerPlatform.h"

extern double s_timeDelta;

struct udViewerInitParams
{
  udViewerInitParams()
  {
    argc = 0;
    argv = NULL;
    renderThreadCount = 0;
  }

  int argc;
  char** argv;
  int renderThreadCount;
};

struct udResizeCallbackData
{
  void *pUserData;
  int width, height;
};

struct udRenderCallbackData
{
  void *pUserData;
  const struct udRenderOptions *pRenderOptions;
  const struct udRenderModel *pRenderModels;
  size_t numRenderModels;
};

enum udViewerCallbackType
{
  udVCT_Update,
  udVCT_Draw,
  udVCT_Resize,

  udVCT_Max
};

typedef void (udViewerCallback)(void *pUserData);

void udViewer_RegisterCallback(udViewerCallbackType type, udViewerCallback *pCallback, void *pUserData);

void udViewer_Init(const udViewerInitParams &initParams);
void udViewer_Deinit();

void udViewer_RunMainLoop();
void udViewer_Quit();

void udViewer_SetRenderModels(struct udRenderModel models[], size_t numModels);
void udViewer_SetRenderOptions(const struct udRenderOptions &options);

#endif
