#pragma once
#if !defined(_EP_STREAMIMPL_H)
#define _EP_STREAMIMPL_H

#include "ep/cpp/component/broadcaster.h"
#include "ep/cpp/component/stream.h"
#include "ep/cpp/internal/i/istream.h"

namespace ep
{

class StreamImpl : public BaseImpl<Stream, IStream>
{
public:
  StreamImpl(Component *pInstance, Variant::VarMap initParams)
    : ImplSuper(pInstance)
  {}

  int64_t Length() const override final { return length; }

  int64_t GetPos() const override final { return pos; }
  int64_t Seek(SeekOrigin rel, int64_t offset) override final { return 0; }

  // TODO: Add efficient Slice<void> support to Variant, so that this component can be shared property?

  Slice<void> Read(Slice<void> buffer) override final { return 0; }
  String ReadLn(Slice<char> buf) override final;
  BufferRef ReadBuffer(size_t bytes) override final;

  int Flush() override final { return 0; }

  BufferRef Load() override final;
  TextRef LoadText() override final;
  void SaveBuffer(BufferRef spBuffer) override final;

  // TODO: support async operations?
  // TODO: remove support for sync operations?

  Variant Save() const override final { return pInstance->Super::Save(); }

protected:
  void SetPos(int64_t _pos) override final { pos = _pos; }
  void SetLength(int64_t _length) override final { length = _length; }

  int64_t length = 0, pos = 0;
};

}

#endif // _EP_STREAMIMPL_H
