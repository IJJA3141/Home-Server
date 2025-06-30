#include "../log.hpp"
#include "server.hpp"

#include <netinet/tcp.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

constexpr int EPOLL_SIZE = 10;

Tcp::Tcp(int _port, const Router& _router)
    : running_(false), router_(_router), addr_(AF_INET, htons(_port), in_addr(htonl(INADDR_ANY)))
{
  // socket
  // int opts[] = {TCP_NODELAY, TCP_COOKIE_TRANSACTIONS};
  // int opts[] = {TCP_COOKIE_TRANSACTIONS};
  int opts[] = {0};

  assert((this->socket_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) != -1);
  assert(setsockopt(this->socket_, SOL_SOCKET, SO_REUSEADDR, &opts, sizeof(opts)) != -1, "failed to set options");
  assert(bind(this->socket_, reinterpret_cast<sockaddr*>(&this->addr_), sizeof(addr_)) != -1);

  // epoll
  epoll_event event(EPOLLIN | EPOLLRDHUP, epoll_data(nullptr)); // might need to bee stored

  assert((this->epoll_ = epoll_create1(0)) != -1);
  assert(epoll_ctl(this->epoll_, EPOLL_CTL_ADD, this->socket_, &event) != -1);

  return;
}

Tcp::~Tcp() { this->close(); }

void Tcp::listen()
{
  assert(::listen(this->socket_, SOMAXCONN) != -1);

  this->running_ = true;
  epoll_event conn_bay[EPOLL_SIZE];

  while (this->running_)
  {
    int n = epoll_wait(this->epoll_, conn_bay, EPOLL_SIZE, -1);
    assert(n != -1);

    log(n, "updates");
    for (int i = 0; i < n; i++)
    {
      Client* client = static_cast<Client*>(conn_bay[i].data.ptr);
      bool closed = conn_bay[i].events & EPOLLRDHUP;

      if (client == nullptr)
      {
        if (closed) // this->socket_ was closed
        {
          log("stoped listeninnng");
          this->running_ = false;
          return;
        }

        // add new client
        log("new client");
        this->client_bay_.insert(this->anchor_client());
      }
      else
      {
        if (closed)
        {
          log("client closed");
          this->client_bay_.erase(reinterpret_cast<Client*>(conn_bay[i].data.ptr));
          delete static_cast<Client*>(conn_bay[i].data.ptr);
          continue;
        }

        client->write(this->router_.respond(client->read()));
      }
    }
  }

  return;
}

void Tcp::close()
{
  assert(this->running_);

  this->running_ = false;

  ::close(this->socket_);
  ::close(this->epoll_);

  for (Client* client : this->client_bay_)
    delete client;

  this->client_bay_.clear();

  log("closed server");
  return;
}

Client* Tcp::anchor_client() const { return new Client(this->epoll_, this->socket_, this->addr_); }
