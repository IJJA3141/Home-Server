#include "../log.hpp"
#include "server.hpp"

#include <cerrno>
#include <cstring>
#include <netinet/tcp.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#define ASS_SOCKET_CREATE    "Failed to create non-blocking TCP socket."
#define ASS_SOCKET_OPTS      "Failed to set socket options (SO_REUSEADDR)."
#define ASS_SOCKET_BIND      "Failed to bind socket to the specified address and port."
#define ASS_SOCKET_LISTEN    "Failed to start listening on TCP socket."
#define ASS_SOCKET_CLOSE     "Failed to close TCP socket."
#define ASS_EPOLL_CREATE     "Failed to create epoll instance."
#define ASS_EPOLL_ADD_SOCKET "Failed to add socket to epoll instance."
#define ASS_EPOLL_CLOSE      "Failed to close epoll file descriptor."
#define ASS_EPOLL_WAIT       "epoll_wait failed while waiting for events."

constexpr int EPOLL_SIZE = 10;

Tcp::Tcp(int _port, const Router& _router)
    : running_(false), router_(_router), addr_(AF_INET, htons(_port), in_addr(htonl(INADDR_ANY)))
{
  // socket
  int opts[] = {TCP_NODELAY, TCP_COOKIE_TRANSACTIONS};

  assert((this->socket_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) != -1, ASS_SOCKET_CREATE);
  assert(setsockopt(this->socket_, SOL_SOCKET, SO_REUSEADDR, &opts, sizeof(opts)) != -1, ASS_SOCKET_OPTS);
  assert(bind(this->socket_, reinterpret_cast<sockaddr*>(&this->addr_), sizeof(addr_)) != -1, ASS_SOCKET_BIND);

  // epoll
  epoll_event event(EPOLLIN | EPOLLRDHUP, epoll_data(nullptr));

  assert((this->epoll_ = epoll_create1(0)) != -1, ASS_EPOLL_CREATE);
  assert(epoll_ctl(this->epoll_, EPOLL_CTL_ADD, this->socket_, &event) != -1, ASS_EPOLL_ADD_SOCKET);

  return;
}

Tcp::~Tcp() { this->close(); }

void Tcp::listen()
{
  assert(::listen(this->socket_, SOMAXCONN) != -1, ASS_SOCKET_LISTEN);

  this->running_ = true;
  epoll_event conn_bay[EPOLL_SIZE];

  int i = 0;
  while (this->running_)
  {
    int n = epoll_wait(this->epoll_, conn_bay, EPOLL_SIZE, -1);
    assert(n != -1 || n != EINTR, ASS_EPOLL_WAIT, strerror(errno));

    for (int i = 0; i < n; ++i)
    {
      const Client* client = static_cast<const Client*>(conn_bay[i].data.ptr);
      const bool closed = conn_bay[i].events & EPOLLRDHUP;

      if (client == nullptr) this->server_event(closed);
      else this->client_event(closed, client);
    }
  }

  return;
}

void Tcp::close()
{
  check(Level::WARN, this->running_, "closed while not running");
  this->running_ = false;

  assert(::close(this->socket_) != -1, ASS_SOCKET_CLOSE);
  assert(::close(this->epoll_) != -1, ASS_EPOLL_CLOSE);

  for (const Client* client : this->client_bay_)
    delete client;

  this->client_bay_.clear();
  return;
}

void Tcp::server_event(const bool _closed)
{
  if (_closed) // this->socket_ was closed
  {
    log("stoped listeninnng");
    this->running_ = false;
    return;
  }

  // add new client
  const Client* const client = this->anchor_client();
  if (client->moored)
  {
    this->client_bay_.insert(client);
    log(client->ip, "connected\t", this->client_bay_.size(), "clients connected");
    return;
  }

  delete client;
  return;
}

void Tcp::client_event(const bool _closed, const Client* _client)
{
  if (_closed)
  {
    this->client_bay_.erase(_client);
    log(_client->ip, "disconnected");
    delete _client;
    log(this->client_bay_.size(), "clients connected");
    return;
  }

  _client->write(this->router_.respond(_client->read()));
  return;
}

Client* Tcp::anchor_client() const { return new Client(this->epoll_, this->socket_, this->addr_); }
