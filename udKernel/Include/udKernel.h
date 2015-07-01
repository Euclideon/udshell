#pragma once
#ifndef UDKERNEL_H
#define UDKERNEL_H

#include "udComponent.h"

struct udRenderEngine;
class udBlockStreamer;

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

  static udKernel *CreateInstanceInternal(udInitParams commandLine);
  udResult DestroyInstanceInternal();
};

// Forward declaration of the entry point function
int EntryInit(const char *pCommandString);// Should this be void? Are we going to check the return value and bail?

udResult udKernel_RunMainLoop(udKernel *pKernel);
udResult udKernel_Quit();

#endif // UDKERNEL_H
