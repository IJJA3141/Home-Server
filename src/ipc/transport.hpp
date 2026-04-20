#pragma once

#include "../protocol/http.hpp"
#include "../protocol/protocol.hpp"
#include <netinet/in.h>

namespace ipc
{

template <Protocol P> class TransportServer
{
public:
  TransportServer(const int _port, const protocol::Handler<P>& _handler);
  ~TransportServer();

  void listen();

private:
  bool running_;
  int socket_;
  struct sockaddr_in addr_;
  const protocol::Handler<P> handler_;
};

template <Protocol P> class TransportClient
{
public:
  TransportClient(const int _port, const char* const _ip);
  ~TransportClient();

  void connect();
  P::Response transmit(P::Request _requset);

private:
  int socket_;
  struct sockaddr_in addr_;
};

} // namespace ipc

template class ipc::TransportServer<protocol::HTTP>;
template class ipc::TransportClient<protocol::HTTP>;
