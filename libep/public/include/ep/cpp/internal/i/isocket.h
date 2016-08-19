#pragma once
#if !defined(_EP_ISOCKET_HPP)
#define _EP_ISOCKET_HPP

#include "ep/cpp/component/stream.h"

namespace ep {

SHARED_CLASS(File);

class ISocket
{
public:
  virtual Slice<void> read(Slice<void> buffer) = 0;
  virtual size_t write(Slice<const void> data) = 0;
  virtual int64_t seek(SeekOrigin rel, int64_t offset) = 0;
};

}

#endif // _EP_ISOCKET_HPP
