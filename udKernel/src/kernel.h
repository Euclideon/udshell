#pragma once
#ifndef UDKERNEL_H
#define UDKERNEL_H

#include "components/component.h"
#include "components/lua.h"
#include "components/view.h"
#include "components/uicomponent.h"
#include "components/timer.h"
#include "helpers.h"

#include "3rdparty/FastDelegate.h"
using namespace fastdelegate;

struct udRenderEngine;

namespace ud
{
class LuaState;

class udBlockStreamer;

PROTOTYPE_COMPONENT(View);
PROTOTYPE_COMPONENT(UIComponent);


// TODO: udMessageHandler returns void, should we return some error state??
typedef FastDelegate3<udString , udString , const udVariant &, void> udMessageHandler;

class Kernel
{
  friend class Component;
public:
  static udResult Create(Kernel **ppInstance, udInitParams commandLine, int renderThreadCount = 0);
  udResult Destroy();

  udResult SendMessage(udString target, udString sender, udString message, const udVariant &data);

  void RegisterMessageHandler(udRCString name, udMessageHandler messageHandler);

  // synchronisation
  typedef FastDelegate<void(Kernel*)> MainThreadCallback;
  void DispatchToMainThread(MainThreadCallback callback);
  void DispatchToMainThreadAndWait(MainThreadCallback callback);

  // component registry
  udResult RegisterComponentType(ComponentDesc *pDesc);
  template<typename ComponentType>
  udResult RegisterComponent();

  udResult CreateComponent(udString typeId, udInitParams initParams, ComponentRef *pNewInstance);
  udResult DestroyComponent(ComponentRef *pInstance);

  template<typename T>
  udSharedPtr<T> CreateComponent(udInitParams initParams = nullptr);

  ComponentRef FindComponent(udString uid);

  Renderer *GetRenderer() const { return pRenderer; }

  // script
  LuaRef GetLua() const { return spLua; }
  void Exec(udString code);

  // other functions
  ViewRef GetFocusView() const { return spFocusView; }
  ViewRef SetFocusView(ViewRef spView);
  udResult FormatMainWindow(UIComponentRef spUIComponent);

  udResult RunMainLoop();
  udResult Terminate();

protected:
  struct ComponentType
  {
    const ComponentDesc *pDesc;
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
  udHashMap<ComponentRef> instanceRegistry;
  udHashMap<ForeignInstance> foreignInstanceRegistry;
  udHashMap<MessageHandler> messageHandlers;

  Renderer *pRenderer = nullptr;

  LuaRef spLua = nullptr;

  ViewRef spFocusView = nullptr;
  TimerRef spStreamerTimer = nullptr;

  static Kernel *CreateInstanceInternal(udInitParams commandLine);
  udResult InitInstanceInternal();
  udResult InitRenderInternal();
  udResult DestroyInstanceInternal();

  udResult InitComponents();
  udResult InitRender();
  udResult DeinitRender();

  udResult ReceiveMessage(udString sender, udString message, const udVariant &data);

  int SendMessage(LuaState L);
  void StreamerUpdate();

  template<typename CT>
  static Component *NewComponent(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, udInitParams initParams);
};

template<typename T>
udSharedPtr<T> Kernel::CreateComponent(udInitParams initParams)
{
  ComponentRef c = nullptr;
  udResult r = CreateComponent(T::descriptor.id, initParams, &c);
  if (r != udR_Success)
    return nullptr;
  return static_pointer_cast<T>(c);
}

} //namespace ud
#endif // UDKERNEL_H
