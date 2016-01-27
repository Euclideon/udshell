#pragma once
#if !defined(_EP_STREAM_H)
#define _EP_STREAM_H

#include "ep/cpp/component/broadcaster.h"
#include "ep/cpp/internal/i/istream.h"

namespace ep {

SHARED_CLASS(Stream);

EP_BITFIELD(OpenFlags,
  Read,
  Write
);

EP_ENUM(SeekOrigin,
  Begin,
  CurrentPos,
  End
);

class Stream : public Broadcaster, public IStream
{
  EP_DECLARE_COMPONENT_WITH_IMPL(Stream, IStream, Broadcaster, EPKERNEL_PLUGINVERSION, "Stream desc...")

public:
  int64_t Length() const override { return pImpl->Length(); }

  int64_t GetPos() const override { return pImpl->GetPos(); }
  int64_t Seek(SeekOrigin rel, int64_t offset) override { return pImpl->Seek(rel, offset); }

  // TODO: Add efficient Slice<void> support to Variant, so that this component can be shared property?

  Slice<void> Read(Slice<void> buffer) override { return pImpl->Read(buffer); }
  String ReadLn(Slice<char> buf) override { return pImpl->ReadLn(buf); }
  BufferRef ReadBuffer(size_t bytes) override { return pImpl->ReadBuffer(bytes); }

  virtual int Flush() override { return pImpl->Flush(); }

  BufferRef Load() override { return pImpl->Load(); }
  void Save(BufferRef spBuffer) override { return pImpl->Save(spBuffer); }

  Event<> PosChanged;

  // TODO: support async operations?
  // TODO: remove support for sync operations?

  Variant Save() const override { return pImpl->Save(); }

protected:
  Stream(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Broadcaster(pType, pKernel, uid, initParams)
  {
    pImpl = CreateImpl(initParams);
  }

  void SetPos(int64_t pos) override final { pImpl->SetPos(pos); }
  void SetLength(int64_t length) override final { pImpl->SetLength(length); }
};

}

#endif // _EP_STREAM_H
