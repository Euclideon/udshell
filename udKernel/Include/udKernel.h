#pragma once
#ifndef UDKERNEL_H
#define UDKERNEL_H

#include "udComponent.h"

struct udRenderEngine;
class udBlockStreamer;
class udView;

typedef void (udMessageHandler)(udString senderUID, udString message, udString data, void *pUserData);

class udKernel
{
  friend class udComponent;
public:
  static udResult Create(udKernel **ppInstance, udInitParams commandLine, int renderThreadCount = 0);
  udResult Destroy();

  virtual udResult SendKernelMessage(udString targetUID, udString message, udString data);
  virtual udResult ReceiveKernelMessage(udString senderUID, udString message, udString data);

  void RegisterMessageHandler(udRCString name, udMessageHandler *pMessageHandler, void *pUserData);

  // component registry
  udResult RegisterComponentType(const udComponentDesc *pDesc);

  udResult CreateComponent(udString typeId, udString initParams, udComponent **ppNewInstance);
  udResult DestroyComponent(udComponent **ppInstance);

  udResult SendComponentMessage(udString targetUID, udString sender, udString message, udString data);

  udComponent *Find(udString uid);

  udRenderEngine *GetRenderEngine() const { return pRenderEngine; }

  // other functions
  udView *GetFocusView() const { return pFocusView; }
  udView *SetFocusView(udView *pView);

  udResult RunMainLoop();
  udResult Terminate();

protected:
  struct MessageHandler
  {
    udRCString name;
    udMessageHandler *pHandler;
    void *pUserData;
  };

  struct ForeignInstance
  {
    udRCString localUID;
    udRCString remoteUID;
    udRCString kernelUID;
  };

  udRCString uid;

  udHashMap<const udComponentDesc*> componentRegistry;
  udHashMap<udComponent*> instanceRegistry;
  udHashMap<ForeignInstance> foreignInstanceRegistry;
  udHashMap<MessageHandler> messageHandlers;

  udRenderEngine *pRenderEngine;
  udBlockStreamer *pStreamer;

  udView *pFocusView;

  static udKernel *CreateInstanceInternal(udInitParams commandLine);
  udResult DestroyInstanceInternal();

  udResult InitComponents();
  udResult InitRender();
};


#endif // UDKERNEL_H
