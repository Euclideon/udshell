#pragma once
#ifndef _EP_SOCKET_HPP
#define _EP_SOCKET_HPP

#include "ep/cpp/internal/i/isocket.h"

#include "ep/cpp/component/stream.h"

namespace ep
{

SHARED_CLASS(Socket);

class Socket : public Stream
{
  EP_DECLARE_COMPONENT_WITH_IMPL(ep, Socket, ISocket, Stream, EPKERNEL_PLUGINVERSION, "Socket stream component", 0)
public:

  Slice<void> read(Slice<void> buffer) override { return pImpl->read(buffer); }
  size_t write(Slice<const void> data) override { return pImpl->write(data); }
  int64_t seek(SeekOrigin rel, int64_t offset) override { return pImpl->seek(rel, offset); }

protected:
  Socket(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Stream(pType, pKernel, uid, initParams)
  {
    pImpl = createImpl(initParams);
  }
};

}

#endif // _EP_SOCKET_HPP
