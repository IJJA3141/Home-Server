#pragma once

#include "../../common/ring_buffer.hpp"
#include "../../config.hpp"
#include "../ipc.hpp"

namespace ipc
{

namespace tcp
{

class Client : IClient
{
public:
  Client(const std::string& ip, const int port);
  ~Client();

  void transmit(protocol::ipcp::Request request) override final;
  void connect();

  void notify_read();
  void notify_write();
  void notify_half_close();
  void notify_close();
  void notify_error();

private:
  using Buffer = RingBuffer<std::byte, IPC_CLIENT_BUFFER_SIZE>;

  std::string ip_;
  int port_;
  int socket_;
  struct sockaddr_in addr_;
  Buffer out_;
  Buffer in_;

  protocol::ipcp::Response::ParserContext ctx_;
  size_t remaining;

  bool recv();
};

class Server
{
};

}; // namespace tcp
} // namespace ipc
