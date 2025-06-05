#include "server.hpp"
#include "../log.hpp"

#include <unistd.h>

Tcp::Tcp( const Router *_router) : router_(_router)
{
  if (_router == nullptr) {
    ERR("Tcp was initialized without a parser");
    exit(1);
  }

  LOG("server initialization...");
  LOG("socket initialization...");

  this->socket_ = socket(AF_INET, SOCK_STREAM, 0);
  if (this->socket_ == -1) {
    VERBERR("socket creation failed.");
    exit(1);
  }

  LOG("setting options...");
  int opt = 1;
  if (setsockopt(this->socket_, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
    VERBERR("setting options failed.");
    exit(1);
  }

  return;
}

Tcp::~Tcp()
{
  ::close(this->socket_);
  this->thread.join();

  for (int i = 0; i < CLIENT_SIZE; i++)
    if (this->client_array_[i] != nullptr) delete this->client_array_[i];

  return;
}

void Tcp::bind(const int _port)
{
  LOG("binding socket to sockaddr on port " << _port << "...");

  this->hint_.sin_family = AF_INET;
  this->hint_.sin_addr.s_addr = htonl(INADDR_ANY);
  this->hint_.sin_port = htons(_port);
  if (::bind(this->socket_, (struct sockaddr *)&this->hint_, sizeof(this->hint_)) == -1) {
    VERBERR("IP/PORT binding failed.");
    exit(1);
  }

  return;
}

void Tcp::listen()
{
  LOG("mark socket for listening...");
  if (::listen(this->socket_, SOMAXCONN) == -1) {
    VERBERR("listening failed.");
    exit(1);
  }

  this->thread = std::thread([this]() -> void {
    while(true) {
      Client *incoming_client = this->await_client();


    }

    //
    //
    //

    LOG("socket stoped listening...");
    return;
  });

  return;
}

Client *Tcp::await_client() { return new Client(this->socket_); }

void Tcp::connect()
{
  // need to connect client
};
