#pragma once
#if !defined(_UD_STREAM_H)
#define _UD_STREAM_H

#include "component.h"
#include "resources/buffer.h"

namespace ud
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
  UD_COMPONENT(Stream);

  int64_t Length() const { return length; }

  int64_t GetPos() const { return pos; }
  virtual int64_t Seek(SeekOrigin rel, int64_t offset) { return 0; }

  virtual size_t Read(void *pData, size_t bytes) { return 0; }
  virtual size_t Write(const void *pData, size_t bytes) { return 0; }

  virtual size_t WriteLn(udString str);
  virtual udString ReadLn(udSlice<char> buf);

  virtual int Flush() { return 0; }

  BufferRef ReadBuffer(size_t bytes);
  size_t WriteBuffer(BufferRef spData);

  BufferRef Load();
  void Save(BufferRef spBuffer);

  // TODO: support async operations?
  // TODO: remove support for sync operations?

protected:
  Stream(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams)
    : Component(pType, pKernel, uid, initParams), pos(0)
  {}

  int64_t length, pos;
};

}

#endif // _UD_STREAM_H
