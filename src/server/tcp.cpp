#include "../log.hpp"
#include "server.hpp"

#include <unistd.h>

TcpServer::TcpServer()
{
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

void TcpServer::bind(const int _port)
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

void TcpServer::listen()
{
  LOG("mark socket for listening...");
  if (::listen(this->socket_, SOMAXCONN) == -1) {
    VERBERR("listening failed.");
    exit(1);
  }

  LOG("received call...");

  while (true) {
    this->threads.push_back(new std::thread(&TcpServer::connect, this, this->newClient_()));
  };

  return;
}

void TcpServer::connect(Client *_client)
{
  while (true) {
    // wait for client new message
    size_t bytes = _client->read();

    if (bytes < 0) {
      VERBERR("failed to read client's message.");
      // should respond
      break;
    }

    // should read client responce
    // and then respond accordingly
  }

  delete _client;

  return;
}

Client *TcpServer::newClient_() { return new Client(this->socket_); }

TcpServer::~TcpServer()
{
  ::close(this->socket_);
  return;
}
