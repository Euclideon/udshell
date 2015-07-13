#pragma once
#ifndef UDKERNEL_H
#define UDKERNEL_H

#include "udComponent.h"
#include "udView.h"
#include "udHelpers.h"

#include "3rdparty\FastDelegate.h"
using namespace fastdelegate;


struct udRenderEngine;
class udBlockStreamer;
PROTOTYPE_COMPONENT(udView);

// TODO: udMessageHandler returns void, should we return some error state??
typedef FastDelegate3<udString , udString , udVariant , void> udMessageHandler;

class udKernel
{
  friend class udComponent;
public:
  static udResult Create(udKernel **ppInstance, udInitParams commandLine, int renderThreadCount = 0);
  udResult Destroy();

  udResult SendMessage(udString target, udString sender, udString message, udVariant data);
  udResult ReceiveMessage(udString sender, udString message, udVariant data);

  void RegisterMessageHandler(udRCString name, udMessageHandler messageHandler);

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
    udMessageHandler callback;
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
