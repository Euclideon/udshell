#pragma once
#if !defined(_EP_STREAM_H)
#define _EP_STREAM_H

#include "broadcaster.h"
#include "resources/buffer.h"

namespace kernel
{
PROTOTYPE_COMPONENT(Stream);

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
  EP_DECLARE_COMPONENT(Stream, Broadcaster, EPKERNEL_PLUGINVERSION, "Stream desc...")
public:

  int64_t Length() const { return length; }

  int64_t GetPos() const { return pos; }
  virtual int64_t Seek(SeekOrigin rel, int64_t offset) { return 0; }

  virtual Slice<void> Read(Slice<void> buffer) { return 0; }
  virtual String ReadLn(Slice<char> buf);
  BufferRef ReadBuffer(size_t bytes);

  virtual int Flush() { return 0; }

  BufferRef Load();
  void Save(BufferRef spBuffer);

  Event<> PosChanged;

  // TODO: support async operations?
  // TODO: remove support for sync operations?

protected:
  Stream(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams)
    : Broadcaster(pType, pKernel, uid, initParams), pos(0)
  {}

  int64_t length, pos;
};

}

#endif // _EP_STREAM_H
