//
#include "./tcp-server.h"

// linux
#include <unistd.h>

// std
#include <iostream>
#include <sstream>

void log(const std::string &_message) {
  std::cout << _message << std::endl;
  return;
}

void error(const std::string &_message) {
  std::cout << "ERROR: " << _message << std::endl;
  exit(1);
  return;
}

http::TcpServer::TcpServer(std::string _ip_address, int _port) {
  this->socket_ = socket(AF_INET, SOCK_STREAM, 0);
  this->ip_address_ = _ip_address;
  this->port_ = _port;
  this->new_socket_ = 0;
  this->incomingMessage_ = 0;
  this->socketAddress_ = sockaddr_in();
  this->socketAddress_len_ = sizeof(this->socketAddress_);
  this->serverMessage_ = buildResponse();

  if (this->socket_ < 0)
    error("Cannot create socket");

  return;
};

http::TcpServer::~TcpServer() {
  close(this->socket_);
  close(this->new_socket_);
  exit(0);

  return;
}
