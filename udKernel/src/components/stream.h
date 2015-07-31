#pragma once
#if !defined(_UD_STREAM_H)
#define _UD_STREAM_H

#include "component.h"

namespace ud
{

enum class SeekOrigin
{
  Begin,
  CurrentPos,
  End
};

class Stream : public Component
{
public:
  UD_COMPONENT(Stream);

  virtual int64_t Length() const { return 0; }

  virtual size_t Read(void *pData, size_t bytes) { return 0; }
  virtual size_t Write(void *pData, size_t bytes) { return 0; }

  virtual int64_t GetPos() { return 0; }
  virtual int64_t Seek(SeekOrigin rel, int64_t offset) { return 0; }

  // TODO: support async operations?
  // TODO: remove support for sync operations?

protected:
  Stream(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, udInitParams initParams)
    : Component(pType, pKernel, uid, initParams)
  {}
};

}

#endif // _UD_STREAM_H
