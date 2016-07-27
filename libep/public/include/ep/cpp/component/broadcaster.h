#pragma once
#if !defined(_EP_BROADCASTER_H)
#define _EP_BROADCASTER_H

#include "ep/cpp/component/component.h"
#include "ep/cpp/internal/i/ibroadcaster.h"
#include "ep/cpp/component/resource/buffer.h"

namespace ep {

SHARED_CLASS(Broadcaster);

class Broadcaster : public Component
{
  EP_DECLARE_COMPONENT_WITH_IMPL(ep, Broadcaster, IBroadcaster, Component, EPKERNEL_PLUGINVERSION, "Broadcast data written to subscribers", 0)

public:
  virtual size_t write(Slice<const void> data) { return pImpl->Write(data); }

  size_t writeLn(String str) { return pImpl->WriteLn(str); }
  size_t writeBuffer(BufferRef spData) { return pImpl->WriteBuffer(spData); }

  Variant save() const override { return pImpl->Save(); }

  Event<Slice<const void>> written;

protected:
  Broadcaster(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Component(pType, pKernel, uid, initParams)
  {
    pImpl = createImpl(initParams);
  }

  Array<const EventInfo> getEvents() const;
};

}

#endif // _EP_BROADCASTER_H
