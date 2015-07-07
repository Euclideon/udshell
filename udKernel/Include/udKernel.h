#pragma once
#ifndef UDKERNEL_H
#define UDKERNEL_H

#include "udComponent.h"
#include "udView.h"


struct udRenderEngine;
class udBlockStreamer;
PROTOTYPE_COMPONENT(udView);

typedef void (udMessageHandler)(udString senderUID, udString message, udString data, void *pUserData);

class udKernel
{
  friend class udComponent;
public:
  static udResult Create(udKernel **ppInstance, udInitParams commandLine, int renderThreadCount = 0);
  udResult Destroy();

  udResult SendMessage(udString target, udString sender, udString message, udString data);
  udResult ReceiveMessage(udString sender, udString message, udString data);

  void RegisterMessageHandler(udRCString name, udMessageHandler *pMessageHandler, void *pUserData);

  // component registry
  udResult RegisterComponentType(const udComponentDesc *pDesc);

  udResult CreateComponent(udString typeId, udString initParams, udComponentRef *pNewInstance);
  udResult DestroyComponent(udComponentRef *pInstance);

  template<typename T>
  udSharedPtr<T> CreateComponent(udString initParams = nullptr);

  udComponentRef Find(udString uid);

  udRenderEngine *GetRenderEngine() const { return pRenderEngine; }

  // other functions
  udViewRef GetFocusView() const { return pFocusView; }
  udViewRef SetFocusView(udViewRef pView);

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
  udHashMap<udComponentRef> instanceRegistry;
  udHashMap<ForeignInstance> foreignInstanceRegistry;
  udHashMap<MessageHandler> messageHandlers;

  udRenderEngine *pRenderEngine = nullptr;

  udViewRef pFocusView;

  static udKernel *CreateInstanceInternal(udInitParams commandLine);
  udResult InitInstanceInternal();
  udResult InitRenderInternal();
  udResult DestroyInstanceInternal();

  udResult InitComponents();
  udResult InitRender();
};

template<typename T>
udSharedPtr<T> udKernel::CreateComponent(udString initParams)
{
  udComponentRef c;
  udResult r = CreateComponent(T::descriptor.id, initParams, &c);
  if (r != udR_Success)
    return udSharedPtr<T>();
  return static_pointer_cast<T>(c);
}

#endif // UDKERNEL_H
