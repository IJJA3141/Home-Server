#include "./tcp-server.h"

// Singleton stuff
http::TcpServer *http::TcpServer::pTcpServer_ = nullptr;

http::TcpServer *http::TcpServer::get() {
  if (http::TcpServer::pTcpServer_ == nullptr)
    http::TcpServer::pTcpServer_ = new http::TcpServer();
  return http::TcpServer::pTcpServer_;
};

http::TcpServer::TcpServer() {
  this->socket_ = 0;
  this->port_ = 0;

  this->clientSize_ = 0;
  this->clientSocket_ = 0;
};

int http::TcpServer::init(std::string _ip, int _port) {
  std::cout << "Creating server socket..." << std::endl;

  this->socket_ = socket(AF_INET, SOCK_STREAM, 0);
  if (this->socket_ == -1) {
    std::cerr << "Socket creating failed.";
    return -1;
  };

  this->hint_.sin_family = AF_INET;
  this->hint_.sin_port = htonl(_port);
  inet_pton(AF_INET, _ip.c_str(), &this->hint_.sin_addr);

  std::cout << "Biding socket to sockaddr..." << std::endl;

  if (bind(this->socket_, (struct sockaddr *)&this->hint_.sin_addr,
           sizeof(this->hint_)) == -1) {
    std::cerr << "IP/PORT binding failed.";
    return -2;
  }

  return 0;
};

int http::TcpServer::listen() {
  std::cout << "Mark the socket for listening..." << std::endl;

  if (::listen(this->socket_, SOMAXCONN) == -1) {
    std::cerr << "Listening failed.";
    return -3;
  }

  this->clientSize_ = sizeof(this->client_);

  std::cout << "Accept client call on PORT: " << this->hint_.sin_port << "..."
            << std::endl;
  this->clientSocket_ = accept(this->socket_, (struct sockaddr *)&this->client_,
                               &this->clientSize_);

  std::cout << "Received call..." << std::endl;
  if (this->clientSocket_ == -1) {
    std::cerr << "Connection with the client failed.";
    return -4;
  }

  // Temp
  std::cout << "Client address: " << inet_ntoa(this->client_.sin_addr)
            << "; PORT: " << this->client_.sin_port << ";" << std::endl;

  close(this->socket_);
  int it = 0;

  while (true) {
    // clear buffer
    memset(this->clientBuffer, 0, 4096);

    // wait for message
    int bytesRecv = recv(this->clientSocket_, this->clientBuffer, 4096, 0);
    if (bytesRecv == -1)
      std::cerr << "Connection issue.";
    if (bytesRecv == 0) {
      std::cout << "Client disconnected." << std::endl;
      break;
    }

    std::cout << "Received:\nit: " << it << "   " << std::string(this->clientBuffer, 0, bytesRecv);

    send(this->clientSocket_, this->clientBuffer, bytesRecv + 1, 0);
  }

  close(this->clientSocket_);

  return 0;
}
