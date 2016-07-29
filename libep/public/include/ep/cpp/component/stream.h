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
  virtual int flush() { return pImpl->Flush(); }

  virtual int64_t length() const { return pImpl->Length(); }

  virtual int64_t getPos() const { return pImpl->GetPos(); }
  virtual int64_t seek(SeekOrigin rel, int64_t offset) { return pImpl->Seek(rel, offset); }

  // TODO: Add efficient Slice<void> support to Variant, so that this component can be shared property?

  virtual Slice<void> read(Slice<void> buffer) { return pImpl->Read(buffer); }

  String readLn(Slice<char> buf) { return pImpl->ReadLn(buf); }
  BufferRef readBuffer(size_t bytes) { return pImpl->ReadBuffer(bytes); }

  BufferRef load() { return pImpl->Load(); }
  TextRef loadText() { return pImpl->LoadText(); }
  void save(BufferRef spBuffer) { return pImpl->SaveBuffer(spBuffer); }

  Event<> posChanged;

  // TODO: support async operations?
  // TODO: remove support for sync operations?

  Variant save() const override { return pImpl->Save(); }

protected:
  Stream(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Broadcaster(pType, pKernel, uid, initParams)
  {
    pImpl = createImpl(initParams);
  }

  void setPos(int64_t pos) { pImpl->SetPos(pos); }
  void setLength(int64_t length) { pImpl->SetLength(length); }
};

}

#endif // _EP_STREAM_H
