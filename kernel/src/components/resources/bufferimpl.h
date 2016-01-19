#pragma once
#ifndef _EP_BUFFERIMPL_HPP
#define _EP_BUFFERIMPL_HPP

#include "ep/cpp/component/resource/buffer.h"
#include "ep/cpp/internal/i/ibuffer.h"

namespace ep {

class BufferImpl : public BaseImpl<Buffer, IBuffer>
{
public:
  BufferImpl(Component *pInstance, Variant::VarMap initParams)
    : Super(pInstance)
  {
  }

  bool Reserve(size_t size) override final;
  bool Allocate(size_t size) override final;
  bool Free() override final;

  bool Empty() const override final { return logicalSize == 0; }
  size_t GetBufferSize() const override final;

  Slice<void> Map() override final;
  Slice<const void> MapForRead() override final;
  void Unmap() override final;

  bool CopyBuffer(BufferRef buffer) override final;
  bool CopyBuffer(Slice<const void> buffer) override final;

  Variant Save() const override final { return pInstance->InstanceSuper::Save(); }

  bool ResizeInternal(size_t size, bool copy) override final;

  Slice<void> buffer;

  size_t logicalSize = 0;
  int mapDepth = 0;
  bool readMap;
};

} // namespace ep

#endif // _EP_BUFFERIMPL_HPP
