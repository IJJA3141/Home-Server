#include "./tcp-server.hpp"
#include <cerrno>

http::Client::Client(const int *_pSocket) {
  this->size = sizeof(this->client);

  this->socket =
      accept(*_pSocket, (struct sockaddr *)&this->client, &this->size);

  if (this->socket == -1) {
    std::cerr << "Connection with the client failed.\n" << errno << std::endl;
    strerror(errno);
    abort();
  }

  return;
}

http::TcpServer::TcpServer() {
  std::cout << "Creating server socket..." << std::endl;

  this->state = State::initializing;

  this->socket_ = socket(AF_INET, SOCK_STREAM, 0);
  if (this->socket_ == -1) {
    std::cerr << "Socket creating failed.";
    this->state = State::brocken;
    return;
  };

  std::cout << "socket: " << this->socket_ << std::endl;

  this->state = State::initialized;
  return;
}

void http::TcpServer::ConnectionHandler_() {
  Client client = Client(&this->socket_);

  while (true) {
    // clear buffer
    memset(client.buffer, 0, 4096);

    // wait for message
    int bytesRecv = recv(client.socket, client.buffer, 4096, 0);
    if (bytesRecv == -1)
      std::cerr << "Connection issue.";
    if (bytesRecv == 0) {
      std::cout << "Client disconnected." << std::endl;
      break;
    }

    std::cout << "Received:\nit: " << std::string(client.buffer, 0, bytesRecv);

    send(client.socket, client.buffer, bytesRecv + 1, 0);
  }

  close(client.socket);

  return;
}

void http::TcpServer::listen(std::string _address, int _port) {
  std::cout << "Biding socket to sockaddr..." << std::endl;
  this->state = State::binding;

  this->hint_.sin_family = AF_INET;
  this->hint_.sin_port = htons(_port);
  inet_pton(AF_INET, _address.c_str(), &this->hint_.sin_addr);

  if (bind(this->socket_, (struct sockaddr *)&this->hint_.sin_addr,
           sizeof(this->hint_)) == -1) {
    std::cerr << "IP/PORT binding failed.";
    this->state = State::brocken;
    return;
  }

  std::cout << "Mark the socket for listening..." << std::endl;

  if (::listen(this->socket_, SOMAXCONN) == -1) {
    std::cerr << "Listening failed.";
    this->state = State::brocken;
    return;
  }

  this->state = State::listening;

  std::cout << "Accept client call on PORT: " << this->hint_.sin_port << "..."
            << std::endl;

  std::cout << "Received call..." << std::endl;

  this->ConnectionHandler_();
  close(this->socket_);
}
