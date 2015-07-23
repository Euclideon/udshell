#pragma once
#ifndef UDKERNEL_H
#define UDKERNEL_H

#include "udComponent.h"
#include "udView.h"
#include "udHelpers.h"

#include "3rdparty\FastDelegate.h"
using namespace fastdelegate;

class LuaState;

struct udRenderEngine;
class udBlockStreamer;
PROTOTYPE_COMPONENT(udView);

// TODO: udMessageHandler returns void, should we return some error state??
typedef FastDelegate3<udString , udString , const udVariant &, void> udMessageHandler;

class udKernel
{
  friend class udComponent;
public:
  static udResult Create(udKernel **ppInstance, udInitParams commandLine, int renderThreadCount = 0);
  udResult Destroy();

  udResult SendMessage(udString target, udString sender, udString message, const udVariant &data);

  void RegisterMessageHandler(udRCString name, udMessageHandler messageHandler);

  // synchronisation
  typedef FastDelegate1<udKernel*, void> MainThreadCallback;
  void DispatchToMainThread(MainThreadCallback callback);
  void DispatchToMainThreadAndWait(MainThreadCallback callback);

  // component registry
  udResult RegisterComponentType(const udComponentDesc *pDesc);

  udResult CreateComponent(udString typeId, udInitParams initParams, udComponentRef *pNewInstance);
  udResult DestroyComponent(udComponentRef *pInstance);

  template<typename T>
  udSharedPtr<T> CreateComponent(udInitParams initParams = nullptr);

  udComponentRef FindComponent(udString uid);

  udRenderEngine *GetRenderEngine() const { return pRenderEngine; }

  // script
  void Exec(udString code);

  // other functions
  udViewRef GetFocusView() const { return spFocusView; }
  udViewRef SetFocusView(udViewRef spView);

  udResult RunMainLoop();
  udResult Terminate();

protected:
  struct ComponentType
  {
    const udComponentDesc *pDesc;
    size_t createCount;
  };
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

  udHashMap<ComponentType> componentRegistry;
  udHashMap<udComponentRef> instanceRegistry;
  udHashMap<ForeignInstance> foreignInstanceRegistry;
  udHashMap<MessageHandler> messageHandlers;

  LuaState *pLua;

  udRenderEngine *pRenderEngine = nullptr;

  udViewRef spFocusView = nullptr;

  static udKernel *CreateInstanceInternal(udInitParams commandLine);
  udResult InitInstanceInternal();
  udResult InitRenderInternal();
  udResult DestroyInstanceInternal();

  udResult InitComponents();
  udResult InitRender();
  udResult DeinitRender();

  udResult ReceiveMessage(udString sender, udString message, const udVariant &data);

  int SendMessage(LuaState L);
};

template<typename T>
udSharedPtr<T> udKernel::CreateComponent(udInitParams initParams)
{
  udComponentRef c = nullptr;
  udResult r = CreateComponent(T::descriptor.id, initParams, &c);
  if (r != udR_Success)
    return nullptr;
  return static_pointer_cast<T>(c);
}

#endif // UDKERNEL_H
