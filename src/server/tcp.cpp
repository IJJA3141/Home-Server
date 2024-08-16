#include "../log.hpp"
#include "server.hpp"

http::TcpServer::TcpServer()
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

void http::TcpServer::bind(const int _port)
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

void http::TcpServer::listen()
{
  LOG("mark socket for listening...");
  if (::listen(this->socket_, SOMAXCONN) == -1) {
    VERBERR("listening failed.");
    exit(1);
  }

  LOG("received call...");

  this->threads.push_back(
      new std::thread(&http::TcpServer::connect, this, this->getClient_(&this->socket_)));

  return;
}

void http::TcpServer::connect(http::Client *_client)
{
  size_t bytes = _client->read();

  if (bytes < 0) {
    VERBERR("failed to read client's message.");
    // should respond
    return;
  } 

  return;
}
