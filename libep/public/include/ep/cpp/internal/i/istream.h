#pragma once
#if !defined(_EP_ISTREAM_HPP)
#define _EP_ISTREAM_HPP

namespace ep {

SHARED_CLASS(Buffer);
SHARED_CLASS(Text);

struct SeekOrigin;
struct OpenFlags;

class IStream
{
public:
  virtual int64_t Length() const = 0;

  virtual int64_t GetPos() const = 0;
  virtual int64_t Seek(SeekOrigin rel, int64_t offset) = 0;

  // TODO: Add efficient Slice<void> support to Variant, so that this component can be shared property?

  virtual Slice<void> Read(Slice<void> buffer) = 0;
  virtual String ReadLn(Slice<char> buf) = 0;
  virtual BufferRef ReadBuffer(size_t bytes) = 0;

  virtual int Flush() = 0;

  virtual BufferRef Load() = 0;
  virtual TextRef LoadText() = 0;
  virtual void SaveBuffer(BufferRef spBuffer) = 0;

  // TODO: support async operations?
  // TODO: remove support for sync operations?

  virtual Variant Save() const = 0;

protected:
  virtual void SetPos(int64_t pos) = 0;
  virtual void SetLength(int64_t length) = 0;
};

}

#endif // _EP_STREAM_H
