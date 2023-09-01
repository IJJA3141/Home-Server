#include "./tcp-server.hpp"
#include "../macro.hpp"
#include <cstring>
#include <string>

int http::TcpServer::serverCount = 0;

http::TcpServer::TcpServer(const char *_name) {
  std::cout << "Server initialization..." << std::endl;
  if (_name == nullptr) {
    this->name = "Server " + std::to_string(http::TcpServer::serverCount);
  } else {
    this->name = _name;
  }

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

void http::TcpServer::bind(const char *_port) {
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

  while (true)
    this->vThread.push_back(new std::thread(http::TcpServer::Connect,
                                            this->GetClient_(&this->socket_),
                                            this->name.c_str()));

  return;
}

void http::TcpServer::Connect(http::Client _client, const char *_name) {
  std::cout << "New client connected." << std::endl;

  size_t bytes = _client.Read();
  if (bytes < 0) {
    std::cerr << "Faild to read client message.\n"
              << strerror(errno) << std::endl;
    _client.Send(HTTP SERVERR END);
  } else {
    _client.buffer[bytes] = '\0';

    std::cout << _name << ":\nClient message:\n" << _client.buffer << std::endl;

    // parse message
    // call adequate function
    // send response
    //
    // paser(_client.buffer);
    // message = lamdafunction[ request ];
    // _client.send(message);
    //
  }

  close(_client.socket);

  return;
}
