#pragma once
#if !defined(_EP_IBROADCASTER_HPP)
#define _EP_IBROADCASTER_HPP

namespace ep {

SHARED_CLASS(Buffer);

class IBroadcaster
{
public:
  virtual size_t Write(Slice<const void> data) = 0;
  virtual size_t WriteLn(String str) = 0;
  virtual size_t WriteBuffer(BufferRef spData) = 0;
  virtual Variant Save() const = 0;
};

}

#endif // _EP_BROADCASTER_H
