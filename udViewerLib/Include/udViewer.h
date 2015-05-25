#pragma once
#ifndef UDVIEWERLIB_H
#define UDVIEWERLIB_H

#include "udViewerPlatform.h"
#include "udRender.h"
#include "udMath.h"

struct udViewerInitParams
{
  udViewerInitParams()
    : pUserData(NULL)
    , argc(0)
    , argv(NULL)
    , renderThreadCount(0)
    , pUpdateCallback(NULL)
    , pRenderCallback(NULL)
    , pResizeCallback(NULL)
    , pReceiveMessage(NULL)
  {}

  // init params
  void *pUserData;

  int argc;
  char** argv;

  int renderThreadCount;

  // callbacks
  void (*pUpdateCallback)(void *pInstanceData);
  void (*pRenderCallback)(const struct udRenderOptions *pRenderOptions, const struct udRenderModel *pRenderModels, size_t numRenderModels, void *pInstanceData);
  void (*pResizeCallback)(int width, int height, void *pInstanceData);
  void (*pReceiveMessage)(const char *pMessage, void *pInstanceData);
};

struct udViewerInstanceData
{
  struct udRenderEngine *pRenderEngine;
  struct udRenderView *pRenderView;

  void *pColorBuffer;
  void *pDepthBuffer;
  int displayWidth, displayHeight;
  int renderWidth, renderHeight;

  double timeDelta;

  void *pUserData;
};

void udViewer_Init(const udViewerInitParams &initParams);
void udViewer_Deinit();

void udViewer_RunMainLoop();
void udViewer_Quit();

void udViewer_SetMatrix(udRenderMatrixType type, const udFloat4x4& proj);

void udViewer_SetRenderModels(struct udRenderModel models[], size_t numModels);
void udViewer_SetRenderOptions(const struct udRenderOptions &options);

const udViewerInstanceData* udViewer_GetInstanceData();

typedef void (udViewer_MessageCallback)(const char *pMessage, void *pUserData);
void udViewer_RegisterMessageHandler(const char *pTargetName, udViewer_MessageCallback *pCallback, void *pUserData);
void udViewer_PostMessage(const char *pTarget, const char *pMessageType, const char *pFormat, ...);

#endif
