#include "../log.hpp"
#include "client.hpp"
#include "http.hpp"
#include "server.hpp"

#include <expected>
#include <map>
#include <memory>
#include <sys/epoll.h>

Tcp::Tcp(const size_t _pool_size) : running_(false), pool_size_(_pool_size), events_(new epoll_event[_pool_size])
{
  this->socket_ = socket(AF_INET, SOCK_STREAM, 0);
  assert(this->socket_ != -1);

  int opt = 1;
  assert(!setsockopt(this->socket_, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)));

  this->epoll_ = epoll_create1(0);
  assert(this->epoll_ != -1);

  return;
}

Tcp::~Tcp() { this->close(); }

bool Tcp::is_running() const { return this->running_; }

void Tcp::bind(const int _port)
{
  this->hint_.sin_family = AF_INET;
  this->hint_.sin_addr.s_addr = htonl(INADDR_ANY);
  this->hint_.sin_port = htons(_port);

  assert(::bind(this->socket_, (struct sockaddr*)&this->hint_, sizeof(this->hint_)) != -1);

  return;
}

void Tcp::listen()
{
  assert(::listen(this->socket_, SOMAXCONN) != -1);
  this->running_ = true;

  this->client_bay_ = std::thread([this]() -> void {
    while (this->running_)
    {
      std::unique_ptr<Client> client = this->await_client();
      this->clients_[*client] = std::move(client);
    }

    return;
  });

  this->client_pool_ = std::thread([this]() -> void {
    while (this->running_)
    {
      int socket_number = epoll_wait(this->epoll_, this->events_, this->pool_size_, -1);

      for (int i = 0; i < socket_number; ++i)
      {
        int client = this->events_[i].data.fd;
        // std::thread(&tcp::handle_client, this, this->events_[i].data.fd);
        std::thread([this, client]() -> void {
          std::expected<Request, Client::Error> req = this->clients_[client]->read();

          if (!req.has_value())
          {
            // if read fails close the client
            this->clients_.erase(client);
          }
          else
          {
          }

          return;
        });
      }
    }

    return;
  });

  return;
}

void Tcp::close()
{
  this->running_ = false;

  ::close(this->socket_);
  ::close(this->epoll_);

  this->client_bay_.join();
  this->client_pool_.join();

  return;
}

std::unique_ptr<Client> Tcp::await_client() { return std::make_unique<Client>(this->socket_, this->epoll_); };
