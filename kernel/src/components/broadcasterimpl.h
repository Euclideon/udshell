#pragma once
#if !defined(_EP_BROADCASTERIMPL_H)
#define _EP_BROADCASTERIMPL_H

#include "ep/cpp/component/broadcaster.h"
#include "ep/cpp/internal/i/ibroadcaster.h"

#include "ep/cpp/component/component.h"
#include "ep/cpp/component/resource/buffer.h"

namespace ep {

class BroadcasterImpl : public BaseImpl<Broadcaster, IBroadcaster>
{
public:
  BroadcasterImpl(Component *pInstance, Variant::VarMap initParams)
    : ImplSuper(pInstance)
  {}

  size_t Write(Slice<const void> data) override final
  {
    pInstance->Written.Signal(data);

    return data.length;
  }

  size_t WriteLn(String str) override final;
  size_t WriteBuffer(BufferRef spData) override final;

  Variant Save() const override final { return pInstance->Super::Save(); }
};

}

#endif // _EP_BROADCASTERIMPL_H
