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
  udKernel *pKernel = CreateInstanceInternal(commandLine);

  pKernel->componentRegistry.Init(256);
  pKernel->instanceRegistry.Init(256);
  pKernel->foreignInstanceRegistry.Init(256);
  pKernel->messageHandlers.Init(64);

  // TODO : make renderThreadCount an optional command string input
//  renderThreadCount = ???;
  udResult result = udRender_Create(&pKernel->pRenderEngine, renderThreadCount);

  const int streamerBuffer = 350*1048576; // TODO : make this an optional command string input
  pKernel->pStreamer = udNew(udBlockStreamer, streamerBuffer);

  // register all the builtin component types
  pKernel->RegisterComponentType(&udComponent::descriptor);
  pKernel->RegisterComponentType(&udView::descriptor);
  pKernel->RegisterComponentType(&udScene::descriptor);
  pKernel->RegisterComponentType(&udNode::descriptor);
  pKernel->RegisterComponentType(&udCamera::descriptor);
  pKernel->RegisterComponentType(&udSimpleCamera::descriptor);
  //...

  *ppInstance = pKernel;
  return udR_Success;
}

udResult udKernel::Destroy()
{
  // unregister components, free stuff
  //...

  messageHandlers.Deinit();
  componentRegistry.Deinit();
  instanceRegistry.Deinit();
  foreignInstanceRegistry.Deinit();

  udResult result = udRender_Destroy(&pRenderEngine);
  udDelete(pStreamer);

  return DestroyInstanceInternal();
}

udResult udKernel::SendKernelMessage(udString targetUID, udString message, udString data)
{
  // default kernel has no friends... :(
  return udR_Success;
}

udResult udKernel::ReceiveKernelMessage(udString senderUID, udString message, udString data)
{
  if (message.length > 3 && message.beginsWithInsensitive("msg"))
  {
    if (message[4] == '@')
    {
      // message is destined for a registered message handler
      // registered message handlers receive messages in the form:
      //   message = "msg@target:subMsg"
      //   data = "data..."

      message.stripFront(4); // strip the "msg@"
      udString target = message.popToken<false>(":"); // target shall preceed a ':'

      MessageHandler *pHandler = messageHandlers.Get(target.toStringz());
      if (pHandler)
        pHandler->pHandler(senderUID, message, data, pHandler->pUserData);
    }
  }

  return udR_Success;
}

void udKernel::RegisterMessageHandler(udRCString name, udMessageHandler *pMessageHandler, void *pUserData)
{
  MessageHandler handler;
  handler.name = name;
  handler.pHandler = pMessageHandler;
  handler.pUserData = pUserData;
  messageHandlers.Add(uid.toStringz(), handler);
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

  // TODO: inform partners that I created a component
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


udResult udKernel::SendComponentMessage(udString targetUID, udString sender, udString message, udString data)
{
  char buffer[64];
  udComponent **ppComponent = instanceRegistry.Get(targetUID.toStringz(buffer, 64));
  if (ppComponent)
  {
    (*ppComponent)->ReceiveMessage(message, sender, data);
    return udR_Success;
  }

  // TODO: check if it's in the foreign component registry and send it there
  //...

  return udR_Failure_; // TODO: better result?
}

udComponent *udKernel::Find(udString uid)
{
  char buffer[64];
  udComponent **ppComponent = instanceRegistry.Get(uid.toStringz(buffer, 64));
  return ppComponent ? *ppComponent : nullptr;
}
