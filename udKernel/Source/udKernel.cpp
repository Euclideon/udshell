#include "udPlatform.h"
#include "udKernel.h"
#include "udRender.h"
#include "udBlockStreamer.h"

#include "udScene.h"
#include "udView.h"
#include "udCamera.h"
#include "udSceneGraph.h"

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
    pKernel->pStreamer = udNew(udBlockStreamer, streamerBuffer);
  }

  // register all the builtin component types
  pKernel->RegisterComponentType(&udComponent::descriptor);
  pKernel->RegisterComponentType(&udView::descriptor);
  pKernel->RegisterComponentType(&udScene::descriptor);
  pKernel->RegisterComponentType(&udNode::descriptor);
  pKernel->RegisterComponentType(&udCamera::descriptor);
  pKernel->RegisterComponentType(&udSimpleCamera::descriptor);
  //...

  // TODO: check out WTF is going on here?!
epilogue:
  *ppInstance = pKernel;

  // init the components
  pKernel->InitComponents();

  // platform init
  pKernel->InitInstanceInternal();

  return udR_Success;
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

  UD_ERROR_CHECK(udRender_Destroy(&pRenderEngine));

  if (pStreamer)
  {
    pStreamer->Destroy();
    udDelete(pStreamer);
  }

  UD_ERROR_CHECK(DestroyInstanceInternal());

epilogue:
  return result;
}

udResult udKernel::SendMessage(udString target, udString sender, udString message, udString data)
{
  if (target.empty())
    return udR_Failure_; // TODO: no target!!

  char targetType = target.popFront();
  if (targetType == '@')
  {
    // component message
    udComponent **ppComponent = instanceRegistry.Get(target.hash());
    if (ppComponent)
    {
      return (*ppComponent)->ReceiveMessage(message, sender, data);
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
      pHandler->pHandler(sender, message, data, pHandler->pUserData);
      return udR_Success;
    }
    else
      return udR_Failure_; // TODO: no message handler
  }

  return udR_Failure_; // TODO: error, invalid target!
}

udResult udKernel::ReceiveMessage(udString sender, udString message, udString data)
{

  return udR_Success;
}

void udKernel::RegisterMessageHandler(udRCString name, udMessageHandler *pMessageHandler, void *pUserData)
{
  MessageHandler handler;
  handler.name = name;
  handler.pHandler = pMessageHandler;
  handler.pUserData = pUserData;
  messageHandlers.Add(name.hash(), handler);
}

udResult udKernel::RegisterComponentType(const udComponentDesc *pDesc)
{
  componentRegistry.Add(pDesc->id, pDesc);
  return udR_Success;
}

udResult udKernel::CreateComponent(udString typeId, udString initParams, udComponent **ppNewInstance)
{
  char buffer[64];
  const udComponentDesc **ppDesc = componentRegistry.Get(typeId.toStringz(buffer, sizeof(buffer)));
  if (!ppDesc)
    return udR_Failure_;

  udComponent *pComponent = nullptr;
  try
  {
    pComponent = (*ppDesc)->pCreateInstance(*ppDesc, this, nullptr, udInitParams());
  }
  catch (udResult r)
  {
    return r;
  }
  catch (...)
  {
    return udR_Failure_;
  }

  if (!pComponent)
    return udR_MemoryAllocationFailure;

  instanceRegistry.Add(pComponent->uid.toStringz(buffer, sizeof(buffer)), pComponent);

  // TODO: inform partner kernels that I created a component
  //...

  *ppNewInstance = pComponent;
  return udR_Success;
}

udResult udKernel::DestroyComponent(udComponent **ppInstance)
{
  // TODO: inform partners that I destroyed a component
  //...

  delete *ppInstance;
  ppInstance = nullptr;
  return udR_Success;
}

udComponent *udKernel::Find(udString uid)
{
  char buffer[64];
  udComponent **ppComponent = instanceRegistry.Get(uid.toStringz(buffer, 64));
  return ppComponent ? *ppComponent : nullptr;
}

udResult udKernel::InitComponents()
{
  udResult r = udR_Success;
  for (auto i : componentRegistry)
  {
    if (i->pInit)
    {
      r = i->pInit();
      if (r != udR_Success)
        break;
    }
  }
  return r;
}

udResult udKernel::InitRender()
{
  udResult r = udR_Success;
  for (auto i : componentRegistry)
  {
    if (i->pInitRender)
    {
      r = i->pInitRender();
      if (r != udR_Success)
        break;
    }
  }
  return r;
}
