#pragma once
#ifndef _EP_BUFFERIMPL_HPP
#define _EP_BUFFERIMPL_HPP

#include "ep/cpp/component/resource/buffer.h"
#include "ep/cpp/internal/i/ibuffer.h"

namespace ep {

class BufferImpl : public BaseImpl<Buffer, IBuffer>
{
public:
  BufferImpl(Component *_pInstance, Variant::VarMap initParams)
    : ImplSuper(_pInstance)
  {
    pInstance->changed.Subscribe(Delegate<void()>(this, &BufferImpl::OnBufferDirty));
  }

  ~BufferImpl()
  {
    pInstance->changed.Unsubscribe(Delegate<void()>(this, &BufferImpl::OnBufferDirty));
  }

  bool Reserve(size_t size) override final;
  bool Allocate(size_t size) override final;
  bool Free() override final;

  bool Empty() const override final { return logicalSize == 0; }
  size_t GetBufferSize() const override final;
  bool Mapped() const override final { return mapDepth > 0; }

  Slice<void> Map() override final;
  Slice<const void> MapForRead() override final;
  void Unmap() override final;

  bool CopyBuffer(BufferRef buffer) override final;
  bool CopySlice(Slice<const void> buffer) override final;

  Variant Save() const override final { return pInstance->Super::save(); }

  bool ResizeInternal(size_t size, bool copy) override final;

  void OnBufferDirty()
  {
    spCachedRenderData = nullptr;
  }

  Slice<void> buffer;
  SharedPtr<RefCounted> spCachedRenderData;
  size_t logicalSize = 0;
  int mapDepth = 0;
  bool readMap;
};

} // namespace ep

#endif // _EP_BUFFERIMPL_HPP
