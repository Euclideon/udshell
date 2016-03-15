#pragma once
#if !defined(_EP_BROADCASTER_H)
#define _EP_BROADCASTER_H

#include "ep/cpp/component/component.h"
#include "ep/cpp/internal/i/ibroadcaster.h"
#include "ep/cpp/component/resource/buffer.h"

namespace ep {

SHARED_CLASS(Broadcaster);

class Broadcaster : public Component, public IBroadcaster
{
  EP_DECLARE_COMPONENT_WITH_IMPL(Broadcaster, IBroadcaster, Component, EPKERNEL_PLUGINVERSION, "Broadcast data written to subscribers", 0)

public:
  size_t Write(Slice<const void> data) override { return pImpl->Write(data); }

  size_t WriteLn(String str) override { return pImpl->WriteLn(str); }
  size_t WriteBuffer(BufferRef spData) override { return pImpl->WriteBuffer(spData); }

  Variant Save() const override { return pImpl->Save(); }

  Event<Slice<const void>> Written;

protected:
  Broadcaster(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Component(pType, pKernel, uid, initParams)
  {
    pImpl = CreateImpl(initParams);
  }
};

}

#endif // _EP_BROADCASTER_H
