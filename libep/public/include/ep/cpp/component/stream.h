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

class Stream : public Broadcaster
{
  EP_DECLARE_COMPONENT_WITH_IMPL(ep, Stream, IStream, Broadcaster, EPKERNEL_PLUGINVERSION, "Stream desc...", 0)

public:
  virtual int Flush() { return pImpl->Flush(); }

  virtual int64_t Length() const { return pImpl->Length(); }

  virtual int64_t GetPos() const { return pImpl->GetPos(); }
  virtual int64_t Seek(SeekOrigin rel, int64_t offset) { return pImpl->Seek(rel, offset); }

  // TODO: Add efficient Slice<void> support to Variant, so that this component can be shared property?

  virtual Slice<void> Read(Slice<void> buffer) { return pImpl->Read(buffer); }

  String ReadLn(Slice<char> buf) { return pImpl->ReadLn(buf); }
  BufferRef ReadBuffer(size_t bytes) { return pImpl->ReadBuffer(bytes); }

  BufferRef Load() { return pImpl->Load(); }
  TextRef LoadText() { return pImpl->LoadText(); }
  void Save(BufferRef spBuffer) { return pImpl->Save(spBuffer); }

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

  void SetPos(int64_t pos) { pImpl->SetPos(pos); }
  void SetLength(int64_t length) { pImpl->SetLength(length); }
};

}

#endif // _EP_STREAM_H
