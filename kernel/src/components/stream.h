#pragma once
#if !defined(_EP_STREAM_H)
#define _EP_STREAM_H

#include "component.h"
#include "resources/buffer.h"

namespace ep
{
PROTOTYPE_COMPONENT(Stream);

UD_BITFIELD(OpenFlags,
  Read,
  Write
);

UD_ENUM(SeekOrigin,
  Begin,
  CurrentPos,
  End
);

class Stream : public Component
{
public:
  EP_COMPONENT(Stream);

  int64_t Length() const { return length; }

  int64_t GetPos() const { return pos; }
  virtual int64_t Seek(SeekOrigin rel, int64_t offset) { return 0; }

  virtual epSlice<void> Read(epSlice<void> buffer) { return 0; }
  virtual size_t Write(epSlice<const void> data) { return 0; }

  virtual epString ReadLn(epSlice<char> buf);
  virtual size_t WriteLn(epString str);

  virtual int Flush() { return 0; }

  BufferRef ReadBuffer(size_t bytes);
  size_t WriteBuffer(BufferRef spData);

  BufferRef Load();
  void Save(BufferRef spBuffer);

  // TODO: support async operations?
  // TODO: remove support for sync operations?

protected:
  Stream(const ComponentDesc *pType, Kernel *pKernel, epSharedString uid, epInitParams initParams)
    : Component(pType, pKernel, uid, initParams), pos(0)
  {}

  int64_t length, pos;
};

}

#endif // _EP_STREAM_H
