#pragma once
#if !defined(_EP_IBUFFER_HPP)
#define _EP_IBUFFER_HPP

#include "ep/cpp/component/component.h"

namespace ep {

SHARED_CLASS(Component);
SHARED_CLASS(Buffer);

class IBuffer
{
public:
  virtual bool Reserve(size_t size) = 0;
  virtual bool Allocate(size_t size) = 0;
  virtual bool Free() = 0;

  virtual bool Empty() const = 0;
  virtual size_t GetBufferSize() const = 0;
  virtual bool Mapped() const = 0;

  virtual Slice<void> Map() = 0;
  virtual Slice<const void> MapForRead() = 0;
  virtual void Unmap() = 0;

  virtual bool CopyBuffer(BufferRef buffer) = 0;
  virtual bool CopySlice(Slice<const void> buffer) = 0;

  virtual Variant Save() const = 0;

protected:
  virtual bool ResizeInternal(size_t size, bool copy) = 0;
};

}

#endif // _EP_IBUFFER_HPP
