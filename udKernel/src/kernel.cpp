#include "udPlatform.h"
#include "kernel.h"
#include "udRender.h"
#include "udBlockStreamer.h"

#include "hal/hal.h"
#include "components/stream.h"
#include "components/file.h"
#include "components/console.h"
#include "components/lua.h"
#include "components/logger.h"
#include "components/timer.h"
#include "components/scene.h"
#include "components/view.h"
#include "components/datasource.h"
#include "components/nodes/node.h"
#include "components/nodes/camera.h"
#include "components/nodes/geomnode.h"
#include "components/nodes/udnode.h"
#include "components/resources/resource.h"
#include "components/resources/buffer.h"
#include "components/resources/array.h"
#include "components/resources/text.h"
#include "components/resources/shader.h"
#include "components/resources/material.h"
#include "components/resources/model.h"
#include "components/resources/udmodel.h"
#include "components/resources/kvpstore.h"
#include "components/resources/metadata.h"
#include "components/datasources/ImageSource.h"
#include "components/datasources/GeomSource.h"
#include "components/datasources/UDDataSource.h"
#include "renderscene.h"
#include "udlua.h"

namespace ud
{

udResult Kernel::Create(Kernel **ppInstance, udInitParams commandLine, int renderThreadCount)
{
  udResult result;
  Kernel *pKernel = CreateInstanceInternal(commandLine);

  UD_ERROR_NULL(pKernel, udR_Failure_);

  pKernel->componentRegistry.Init(256);
  pKernel->instanceRegistry.Init(256);
  pKernel->foreignInstanceRegistry.Init(256);
  pKernel->messageHandlers.Init(64);

  pKernel->pRenderer = udNew(Renderer, pKernel, renderThreadCount);

  // register all the builtin component types
  UD_ERROR_CHECK(pKernel->RegisterComponent<Component>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<DataSource>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<Stream>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<File>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<Console>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<Logger>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<Timer>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<Lua>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<UIComponent>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<View>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<Scene>());

  // nodes
  UD_ERROR_CHECK(pKernel->RegisterComponent<Node>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<Camera>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<SimpleCamera>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<GeomNode>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<UDNode>());

  // resources
  UD_ERROR_CHECK(pKernel->RegisterComponent<Resource>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<Buffer>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<ArrayBuffer>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<UDModel>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<Shader>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<Material>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<Model>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<Text>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<KVPStore>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<Metadata>());

  // data sources
  UD_ERROR_CHECK(pKernel->RegisterComponent<ImageSource>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<GeomSource>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<UDDataSource>());

  //...

  // init the HAL
  UD_ERROR_CHECK(udHAL_Init());

  // platform init
  UD_ERROR_CHECK(pKernel->InitInstanceInternal());

  // init the components
  UD_ERROR_CHECK(pKernel->InitComponents());

  pKernel->spLua = pKernel->CreateComponent<Lua>();

  pKernel->spLogger = pKernel->CreateComponent<Logger>();

  pKernel->spStreamerTimer = pKernel->CreateComponent<Timer>({ { "duration", 33 }, { "timertype", "Interval" } });
  pKernel->spStreamerTimer->Event.Subscribe(FastDelegate<void()>(pKernel, &Kernel::StreamerUpdate));

epilogue:
  if (result != udR_Success)
  {
    // TODO: clean up code
  }
  *ppInstance = pKernel;
  return result;
}

udResult Kernel::Destroy()
{
  udResult result;

  // unregister components, free stuff
  //...

  // TODO: this crashes - fix!
  //udDelete(pLua);

  // TODO: Destroy the streamer timer
  // pKernel->spStreamerTimer;

  messageHandlers.Deinit();
  componentRegistry.Deinit();
  instanceRegistry.Deinit();
  foreignInstanceRegistry.Deinit();

  udOctree_Shutdown();

  delete pRenderer;

  udHAL_Deinit();

  UD_ERROR_CHECK(DestroyInstanceInternal());

epilogue:
  return result;
}

void Kernel::StreamerUpdate()
{
  udStreamerStatus streamerStatus = { 0 };
  udOctree_Update(&streamerStatus);
}

udResult Kernel::SendMessage(udString target, udString sender, udString message, const udVariant &data)
{
  if (target.empty())
    return udR_Failure_; // TODO: no target!!

  char targetType = target.popFront();
  if (targetType == '@')
  {
    // component message
    ComponentRef *pComponent = instanceRegistry.Get(target.hash());
    if (pComponent)
    {
      return (*pComponent)->ReceiveMessage(message, sender, data);
    }
    else
    {
      // TODO: check if it's in the foreign component registry and send it there

      return udR_Failure_; // TODO: no component!
    }
  }
  else if (targetType == '#')
  {
    // kernel message
    if (target.eq(uid))
    {
      // it's for me!
      return ReceiveMessage(sender, message, data);
    }
    else
    {
      // TODO: foreign kernels?!

      return udR_Failure_; // TODO: invalid kernel!
    }
  }
  else if (targetType == '$')
  {
    // registered message
    MessageHandler *pHandler = messageHandlers.Get(target.hash());
    if (pHandler)
    {
      pHandler->callback(sender, message, data);
      return udR_Success;
    }
    else
      return udR_Failure_; // TODO: no message handler
  }

  return udR_Failure_; // TODO: error, invalid target!
}

udResult Kernel::ReceiveMessage(udString sender, udString message, const udVariant &data)
{

  return udR_Success;
}

void Kernel::RegisterMessageHandler(udRCString name, udMessageHandler messageHandler)
{
  MessageHandler handler;
  handler.name = name;
  handler.callback = messageHandler;
  messageHandlers.Add(name.hash(), handler);
}

udResult Kernel::RegisterComponentType(ComponentDesc *pDesc)
{
  if (pDesc->id.canFind('@') || pDesc->id.canFind('$') || pDesc->id.canFind('#'))
  {
    UDASSERT(false, "Invalid component id");
    return udR_Failure_;
  }

  // build search trees
  pDesc->BuildSearchTrees();

  // add to registry
  ComponentType t = { pDesc, 0 };
  componentRegistry.Add(pDesc->id.hash(), t);
  return udR_Success;
}

template<typename CT>
Component *Kernel::NewComponent(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, udInitParams initParams)
{
  return udNew(CT, pType, pKernel, uid, initParams);
}
template<typename CT>
udResult Kernel::RegisterComponent()
{
  if (!CT::descriptor.pCreateInstance)
    CT::descriptor.pCreateInstance = &NewComponent<CT>;
  return RegisterComponentType(&CT::descriptor);
}

udResult Kernel::CreateComponent(udString typeId, udInitParams initParams, ComponentRef *pNewInstance)
{
  ComponentType *pType = componentRegistry.Get(typeId.hash());
  if (!pType)
    return udR_Failure_;

  try
  {
    const ComponentDesc *pDesc = pType->pDesc;

    // TODO: should we have a better uid generator than this?
    udFixedString64 uid = udFixedString64::format("%s%d", pDesc->id.ptr, pType->createCount++);

    ComponentRef spComponent(pDesc->pCreateInstance(pDesc, this, uid, initParams));
    if (!spComponent)
      return udR_MemoryAllocationFailure;

    spComponent->Init(initParams);

    instanceRegistry.Add(spComponent->uid.hash(), spComponent);

    if (spLua)
      spLua->SetGlobal(spComponent, udString(spComponent->uid));

    // TODO: inform partner kernels that I created a component
    //...

    *pNewInstance = spComponent;
    return udR_Success;
  }
  catch (udResult r)
  {
    return r;
  }
  catch (...)
  {
    return udR_Failure_;
  }
}

udResult Kernel::DestroyComponent(ComponentRef *pInstance)
{
  spLua->SetGlobal(nullptr, udString((*pInstance)->uid));

  // TODO: remove from component registry
  //instanceRegistry.Destroy((*pInstance)->uid.toStringz());

  // TODO: inform partners that I destroyed a component
  //...

  pInstance->reset();
  pInstance = nullptr;
  return udR_Success;
}

ComponentRef Kernel::FindComponent(udString uid)
{
  if (uid.empty() || uid[0] == '$' || uid[0] == '#')
    return nullptr;
  if (uid[0] == '@')
    uid.popFront();
  ComponentRef *pComponent = instanceRegistry.Get(uid.toStringz());
  return pComponent ? *pComponent : nullptr;
}

udResult Kernel::InitComponents()
{
  udResult r = udR_Success;
  for (auto i : componentRegistry)
  {
    if (i.pDesc->pInit)
    {
      r = i.pDesc->pInit();
      if (r != udR_Success)
        break;
    }
  }
  return r;
}

void udRenderScene_InitRender();

udResult Kernel::InitRender()
{
  udHAL_InitRender();

  udRenderScene_InitRender();

  return udR_Success;
}

udResult Kernel::DeinitRender()
{
  udHAL_DeinitRender();

  return udR_Success;
}

void Kernel::Exec(udString code)
{
  spLua->Execute(code);
}

// Helper functions for the kernel's logger
void Kernel::LogError(int level, const udString text, const udString componentUID) { spLogger->Log(level, text, LogCategories::Error, componentUID); }
void Kernel::LogWarning(int level, const udString text, const udString componentUID) { spLogger->Log(level, text, LogCategories::Warning, componentUID); }
void Kernel::LogDebug(int level, const udString text, const udString componentUID) { spLogger->Log(level, text, LogCategories::Debug, componentUID); }
void Kernel::LogInfo(int level, const udString text, const udString componentUID) { spLogger->Log(level, text, LogCategories::Info, componentUID); }
void Kernel::LogScript(int level, const udString text, const udString componentUID) { spLogger->Log(level, text, LogCategories::Script, componentUID); }
void Kernel::LogTrace(int level, const udString text, const udString componentUID) { spLogger->Log(level, text, LogCategories::Trace, componentUID); }
// Short name for LogDebug function
void Kernel::Log(int level, const udString text, const udString componentUID) { spLogger->Log(level, text, LogCategories::Debug, componentUID); }

} // namespace ud

// synchronised pointer destroy function (it's here because there's no udsharedptr.cpp file)
template<class T>
void udSynchronisedPtr<T>::destroy()
{
  struct S
  {
    void Destroy(ud::Kernel *pKernel)
    {
      ((udSharedPtr<T>&)this)->release();
    }
  };

  pKernel->DispatchToMainThread(MakeDelegate((S*)this, &S::Destroy));
  pInstance = nullptr;
}

