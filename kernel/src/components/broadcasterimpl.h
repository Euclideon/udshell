#pragma once
#if !defined(_EP_BROADCASTERIMPL_H)
#define _EP_BROADCASTERIMPL_H

#include "ep/cpp/component/broadcaster.h"
#include "ep/cpp/internal/i/ibroadcaster.h"

#include "ep/cpp/component/component.h"
#include "ep/cpp/component/resource/buffer.h"

namespace ep {
inline Array<const EventInfo> Broadcaster::getEvents() const
{
  return{
    EP_MAKE_EVENT(written, "Data was written")
  };
}
class BroadcasterImpl : public BaseImpl<Broadcaster, IBroadcaster>
{
public:
  BroadcasterImpl(Component *pInstance, Variant::VarMap initParams)
    : ImplSuper(pInstance)
  {}

  size_t Write(Slice<const void> data) override final
  {
    pInstance->written.Signal(data);

    return data.length;
  }

  size_t WriteLn(String str) override final;
  size_t WriteBuffer(BufferRef spData) override final;

  Variant Save() const override final { return pInstance->Super::save(); }
};

}

#endif // _EP_BROADCASTERIMPL_H
