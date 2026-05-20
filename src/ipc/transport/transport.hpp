#pragma once

#include "../../config.hpp"
#include "../../protocol/atp/atp.hpp"
#include "../../protocol/http/http.hpp"
#include "../../protocol/protocol.hpp"
#include "../../utils/buffer.hpp"
#include <netinet/in.h>
#include <string>
#include <sys/types.h>

namespace ipc
{

/**
 *  @brief IPC over TCP server side
 *
 *  @tparam P protocol used by the server/client IPC
 *
 *  This class is used to receive request from other processes.
 */
template <protocol::Protocol P> class TransportServer
{
  using ParserContext = typename P::Request::ParserContext;

public:
  /**
   *  @brief Creates a %TransportServer
   *
   *  @param ip  server listening IP
   *  @param port  server listening port
   *  @param handler  request -> response handler
   *  @param bad_request  response send on parser failure
   *
   *  @throws ...
   *
   *  Creates a TCP socket listening on @a ip : @a port. On successful @a
   *  P::Request reception and parsing, calls @a handler to compute and send a
   *  response. On failure send @a bad_request.
   */
  TransportServer(const std::string& ip, const int port, const protocol::Handler<P> handler,
                  const std::string bad_request);

  ~TransportServer();

  /**
   *  @brief start listening loop
   *
   *  Marks the internal socket as listening and handles clients connections
   *  and requests.
   */
  void listen();

private:
  struct Client
  {
    const int socket;
    RingBuffer<char, BUFFER_SIZE> connection_buffer;
    ParserContext parsing_ctx;
    char ip[INET_ADDRSTRLEN];
    int port;
  };

  bool listening_;
  int socket_;
  struct sockaddr_in addr_;
  const protocol::Handler<P> handler_;
  const std::string bad_request_;

  // keep for logging
  const std::string ip_;
  const int port_;
};

/**
 *  @brief IPC over TCP client side
 *
 *  @tparam P Protocol
 *
 *  This class is used to send request to other processes.
 */
template <protocol::Protocol P> class TransportClient
{
public:
  /**
   *  @brief  Creates a %TransportClient
   *
   *  @param _ip  @a TransportServer IP
   *  @param _port  @a TransportServer port
   *
   *  @throws ...
   *
   */
  TransportClient(const std::string ip, const int port);
  ~TransportClient();

  /**
   *  @brief connects this %TransportClient to the @a TransportServer
   *
   *  @throws ...
   */
  void connect();

  /**
   *  @brief Transmit @param request to connect @a TransportServer and gets a
   *  response.
   *
   *  @param _request  Request to be send.
   *
   *  @return  The response send by the @a TransportServer.
   *
   *  @throws ...
   *
   *  Sends a request from protocol P to connected @a TransportServer and gets
   *  it's response from protocol P. Since this function assumes that the
   *  response will be valid, it throws if any failure accures. The request on
   *  the other hand does not need to be valid.
   */
  P::Response transmit(const P::Request& request);

private:
  int socket_;
  const std::string ip_;
  const int port_;
  struct sockaddr_in addr_;
  RingBuffer<char, BUFFER_SIZE> buffer_;
  P::Response::ParserContext parser_ctx_;
};

// Template instantiation for hyper text transfer protocol.
template class TransportServer<protocol::HTTP>;
template class TransportClient<protocol::HTTP>;

template class TransportServer<protocol::ATP>;
template class TransportClient<protocol::ATP>;

} // namespace ipc
