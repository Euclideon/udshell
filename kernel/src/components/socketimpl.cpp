#include "components/socketimpl.h"
#include "components/streamimpl.h"

#if defined(EP_WINDOWS)
# include <Ws2tcpip.h>
# pragma comment(lib, "Ws2_32.lib")
#elif defined(EP_LINUX)
# include <unistd.h>
# include <fcntl.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netdb.h>
#else
# error "not supported!"
#endif

namespace ep {

#if defined(EP_WINDOWS)
  bool SocketImpl::bInitialised = false;
  static WSADATA wsaData;
#endif

SocketImpl::SocketImpl(Component *_pInstance, Variant::VarMap initParams)
  : ImplSuper(_pInstance)
{
#if defined(EP_WINDOWS)
  if (!bInitialised)
  {
    int r = WSAStartup(MAKEWORD(2, 2), &wsaData);
    EPTHROW_IF(r != 0, Result::Failure, "WSAStartup failed: {0}", r);
    bInitialised = true;
  }
#endif

  const Variant *pV = initParams.get("url");
  if (!pV)
    EPTHROW_ERROR(Result::InvalidArgument, "Missing 'url'");
  SharedString url = pV->asSharedString();

  pV = initParams.get("port");
  if (!pV)
    EPTHROW_ERROR(Result::InvalidArgument, "Missing 'port'");
  SharedString port = pV->asSharedString();

  addrinfo *result = nullptr, *ptr = nullptr, hints;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  // Resolve the server address and port
  int r = getaddrinfo(url.toStringz(), port.toStringz(), &hints, &result);
  EPTHROW_IF(r != 0, Result::Failure, "getaddrinfo failed: {0}", r);

  ptr = result;

  sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
  if (sock == INVALID_SOCKET)
  {
    freeaddrinfo(result);
#if defined(EP_WINDOWS)
    EPTHROW(Result::Failure, "Couldn't create socket: {0}", WSAGetLastError());
#else
    EPTHROW(Result::Failure, "Couldn't create socket: {0}", errno);
#endif
  }

  r = connect(sock, ptr->ai_addr, (int)ptr->ai_addrlen);
  if (r == SOCKET_ERROR)
  {
    // TODO: recurse the list of addresses returned by getaddrinfo

#if defined(EP_WINDOWS)
    closesocket(sock);
#else
    close(sock);
#endif
    EPTHROW(Result::File_OpenFailure, "Couldn't connect to server {0}", url);
  }

  freeaddrinfo(result);

  // set non-blocking
#if defined(EP_WINDOWS)
  u_long mode = 1;
  ioctlsocket(sock, FIONBIO, &mode);
#elif defined(EP_LINUX)
  r = fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK);
  if (r == -1)
    EPTHROW_IF(r == -1, Result::Failure, "Error calling fcntl!");
#else
# error
#endif
}

SocketImpl::~SocketImpl()
{
#if defined(EP_WINDOWS)
  closesocket(sock);
#else
  close(sock);
#endif
}

Slice<void> SocketImpl::read(Slice<void> buffer)
{
  if (sock == INVALID_SOCKET)
  {
    // TODO: warn that there is no active connection?
    return nullptr;
  }

  int r = recv(sock, (char*)buffer.ptr, (int)buffer.length, 0);
  if (r > 0)
  {
    return buffer.slice(0, r);
  }
  else if (r == 0)
  {
    sock = INVALID_SOCKET;
    logWarning(2, "Connection was closed by remote");
  }
  else
  {
#if defined(EP_WINDOWS)
    int err = WSAGetLastError();
    if (err == WSAEWOULDBLOCK)
      return nullptr;
    EPTHROW(Result::Failure, "Socket recv failed: {0}", err);
#else
    if (errno == EAGAIN)
      return nullptr;
    EPTHROW(Result::Failure, "Socket send failed: {0}", errno);
#endif
  }
  return nullptr;
}

size_t SocketImpl::write(Slice<const void> data)
{
  if (sock == INVALID_SOCKET)
  {
    // TODO: warm that there is no active connection?
    return 0;
  }

  int r = send(sock, (const char*)data.ptr, (int)data.length, 0);
  if (r > 0)
  {
    pInstance->Broadcaster::write(data.slice(0, r));
    return r;
  }
  else if (r == 0)
  {
    sock = INVALID_SOCKET;
    logWarning(2, "Connection was closed by remote");
  }
  else
  {
#if defined(EP_WINDOWS)
    int err = WSAGetLastError();
    if (err == WSAEWOULDBLOCK)
      return 0;
    EPTHROW(Result::Failure, "Socket send failed: {0}", err);
#else
    if (errno == EAGAIN)
      return 0;
    EPTHROW(Result::Failure, "Socket send failed: {0}", errno);
#endif
  }
  return 0;
}

int64_t SocketImpl::seek(SeekOrigin rel, int64_t offset)
{
  EPTHROW(Result::InvalidCall, "Can't seek socket streams!");
}

} // namespace ep
