#pragma once
#if !defined(_EP_BROADCASTER_H)
#define _EP_BROADCASTER_H

#include "component.h"
#include "resources/buffer.h"

namespace kernel
{
PROTOTYPE_COMPONENT(Broadcaster);

class Broadcaster : public Component
{
  EP_DECLARE_COMPONENT(Broadcaster, Component, EPKERNEL_PLUGINVERSION, "Broadcast data written to subscribers");
public:

  virtual size_t Write(Slice<const void> data)
  {
    Written.Signal(data);

    return data.length;
  }

  virtual size_t WriteLn(String str);
  size_t WriteBuffer(BufferRef spData);

  Event<Slice<const void>> Written;

protected:
  Broadcaster(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams)
    : Component(pType, pKernel, uid, initParams)
  {}
};

}

#endif // _EP_BROADCASTER_H
