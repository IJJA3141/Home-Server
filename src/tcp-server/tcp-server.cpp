#include "./tcp-server.hpp"

http::Client::Client(const int *_pSocket) {
  this->size = sizeof(this->client);

  this->socket =
      accept(*_pSocket, (struct sockaddr *)&this->client, &this->size);

  if (this->socket == -1) {
    std::cerr << "Connection with the client failed.\n"
              << strerror(errno) << std::endl;
    abort();
  }

  return;
}

http::TcpServer::TcpServer() {
  std::cout << "Creating server socket..." << std::endl;
  this->socket_ = socket(AF_INET, SOCK_STREAM, 0);
  if (this->socket_ == -1) {
    std::cerr << "Socket creating failed.\n" << strerror(errno) << std::endl;
    abort();
  };

  std::cout << "Setting server options..." << std::endl;
  int opt = 1;
  if (setsockopt(this->socket_, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt))) {
    std::cerr << "Setting options failed.\n" << strerror(errno) << std::endl;
    abort();
  }

  return;
}

void http::TcpServer::bind(char *_port) {
  std::cout << "Biding socket to sockaddr on port: " << _port << "..."
            << std::endl;
  this->hint_.sin_family = AF_INET;
  this->hint_.sin_addr.s_addr = inet_addr(_port);
  this->hint_.sin_port = htons(0);
  if (::bind(this->socket_, (struct sockaddr *)&this->hint_.sin_addr,
             sizeof(this->hint_)) == -1) {
    std::cerr << "IP/PORT binding failed.\n" << strerror(errno) << std::endl;
    abort();
  }

  return;
}

void http::TcpServer::listen() {
  std::cout << "Mark the socket for listening..." << std::endl;
  if (::listen(this->socket_, SOMAXCONN) == -1) {
    std::cerr << "Listening failed.\n" << strerror(errno) << std::endl;
    abort();
  }

  std::cout << "Received call..." << std::endl;
  this->ConnectionHandler_();
  close(this->socket_);
}

void http::TcpServer::ConnectionHandler_() {
  Client client = Client(&this->socket_);
  int bytesRecv = recv(client.socket, client.buffer, 4096, 0);

  send(client.socket, "HTTP/1.1 301 Moved Permanently",
       strlen("HTTP/1.1 301 Moved Permanently"), 0);
  send(client.socket, "Location: https://ijja.dev",
       strlen("Location: https://ijja.dev"), 0);

  close(client.socket);

  return;
}
