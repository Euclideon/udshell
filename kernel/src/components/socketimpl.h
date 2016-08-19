#pragma once
#if !defined(_EP_SOCKETIMPL_H)
#define _EP_SOCKETIMPL_H

#include "ep/cpp/component/socket.h"

#if defined(EP_WINDOWS)
# include <winsock2.h>
#elif defined(EP_LINUX)
  using SOCKET = int;
# define INVALID_SOCKET -1
# define SOCKET_ERROR -1
#endif

namespace ep {

SHARED_CLASS(File);

class SocketImpl : public BaseImpl<Socket, ISocket>
{
public:
  SocketImpl(Component *pInstance, Variant::VarMap initParams);
  ~SocketImpl();

  Slice<void> read(Slice<void> buffer) override final;
  size_t write(Slice<const void> data) override final;
  int64_t seek(SeekOrigin rel, int64_t offset) override final;

protected:
  SOCKET sock = INVALID_SOCKET;

#if defined(EP_WINDOWS)
  static bool bInitialised;
#endif
};

}

#endif // _EP_SOCKETIMPL_H
