#include "udPlatform.h"
#include "udKernel.h"
#include "udRender.h"
#include "udBlockStreamer.h"

#include "udHAL.h"
#include "udScene.h"
#include "udView.h"
#include "udCamera.h"
#include "udSceneGraph.h"

#include "udLua.h"


udResult udKernel::Create(udKernel **ppInstance, udInitParams commandLine, int renderThreadCount)
{
  udResult result;
  udKernel *pKernel = CreateInstanceInternal(commandLine);
  const int streamerBuffer = 550*1048576; // TODO : make this an optional command string input

  UD_ERROR_NULL(pKernel, udR_Failure_);

  pKernel->componentRegistry.Init(256);
  pKernel->instanceRegistry.Init(256);
  pKernel->foreignInstanceRegistry.Init(256);
  pKernel->messageHandlers.Init(64);

  // TODO: Remove this once webview is properly integrated
  if (renderThreadCount != -1)
  {
    UD_ERROR_CHECK(udRender_Create(&pKernel->pRenderEngine, renderThreadCount));
    UD_ERROR_CHECK(udOctree_Init(streamerBuffer));
  }

  // register all the builtin component types
  UD_ERROR_CHECK(pKernel->RegisterComponentType(&udComponent::descriptor));
  UD_ERROR_CHECK(pKernel->RegisterComponentType(&udView::descriptor));
  UD_ERROR_CHECK(pKernel->RegisterComponentType(&udScene::descriptor));
  UD_ERROR_CHECK(pKernel->RegisterComponentType(&udNode::descriptor));
  UD_ERROR_CHECK(pKernel->RegisterComponentType(&udCamera::descriptor));
  UD_ERROR_CHECK(pKernel->RegisterComponentType(&udSimpleCamera::descriptor));
  //...

  // init the HAL
  UD_ERROR_CHECK(udHAL_Init());

  // platform init
  UD_ERROR_CHECK(pKernel->InitInstanceInternal());

  // init the components
  UD_ERROR_CHECK(pKernel->InitComponents());

  pKernel->pLua = new LuaState(pKernel);

epilogue:
  if (result != udR_Success)
  {
    // TODO: clean up code
  }
  *ppInstance = pKernel;
  return result;
}

udResult udKernel::Destroy()
{
  udResult result;

  // unregister components, free stuff
  //...

  messageHandlers.Deinit();
  componentRegistry.Deinit();
  instanceRegistry.Deinit();
  foreignInstanceRegistry.Deinit();

  udOctree_Shutdown();

  UD_ERROR_CHECK(udRender_Destroy(&pRenderEngine));

  udHAL_Deinit();

  UD_ERROR_CHECK(DestroyInstanceInternal());

epilogue:
  return result;
}

udResult udKernel::SendMessage(udString target, udString sender, udString message, udVariant data)
{
  if (target.empty())
    return udR_Failure_; // TODO: no target!!

  char targetType = target.popFront();
  if (targetType == '@')
  {
    // component message
    udComponentRef *pComponent = instanceRegistry.Get(target.hash());
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

udResult udKernel::ReceiveMessage(udString sender, udString message, udVariant data)
{

  return udR_Success;
}

void udKernel::RegisterMessageHandler(udRCString name, udMessageHandler messageHandler)
{
  MessageHandler handler;
  handler.name = name;
  handler.callback = messageHandler;
  messageHandlers.Add(name.hash(), handler);
}

udResult udKernel::RegisterComponentType(const udComponentDesc *pDesc)
{
  if (pDesc->id.canFind('@') || pDesc->id.canFind('$') || pDesc->id.canFind('#'))
  {
    UDASSERT(false, "Invalid component id");
    return udR_Failure_;
  }

  ComponentType t = { pDesc, 0 };
  componentRegistry.Add(pDesc->id.hash(), t);
  return udR_Success;
}

udResult udKernel::CreateComponent(udString typeId, udInitParams initParams, udComponentRef *pNewInstance)
{
  ComponentType *pType = componentRegistry.Get(typeId.hash());
  if (!pType)
    return udR_Failure_;

  try
  {
    const udComponentDesc *pDesc = pType->pDesc;

    // TODO: should we have a better uid generator than this?
    udFixedString64 uid = udFixedString64::format("%s%d", pDesc->id.ptr, pType->createCount++);

    udComponentRef pComponent(pDesc->pCreateInstance(pDesc, this, uid, initParams));

    if (!pComponent)
      return udR_MemoryAllocationFailure;

    instanceRegistry.Add(pComponent->uid.hash(), pComponent);

    pLua->setComponent(pComponent, udString(pComponent->uid));

    // TODO: inform partner kernels that I created a component
    //...

    *pNewInstance = pComponent;
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

udResult udKernel::DestroyComponent(udComponentRef *pInstance)
{
  pLua->setNil(udString((*pInstance)->uid));

  // TODO: remove from component registry
//  instanceRegistry.Destroy(...);

  // TODO: inform partners that I destroyed a component
  //...

  pInstance->reset();
  pInstance = nullptr;
  return udR_Success;
}

udComponentRef udKernel::FindComponent(udString uid)
{
  if (uid.empty() || uid[0] == '$' || uid[0] == '#')
    return udComponentRef();
  if (uid[0] == '@')
    uid.popFront();
  udComponentRef *pComponent = instanceRegistry.Get(uid.toStringz());
  return pComponent ? *pComponent : udComponentRef();
}

udResult udKernel::InitComponents()
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

udResult udKernel::InitRender()
{
  udHAL_InitRender();

  udResult r = udR_Success;
  for (auto i : componentRegistry)
  {
    if (i.pDesc->pInitRender)
    {
      r = i.pDesc->pInitRender();
      if (r != udR_Success)
        break;
    }
  }
  return r;
}

void udKernel::Exec(udString code)
{
  pLua->exec(code);
}
