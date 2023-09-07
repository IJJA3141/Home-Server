#include "./tcp-server.hpp"
#include <netinet/in.h>

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
  this->hint_.sin_addr.s_addr = htonl(INADDR_ANY);
  this->hint_.sin_port = htons(std::atoi(_port));
  if (::bind(this->socket_, (struct sockaddr *)&this->hint_,
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
    this->vThread.push_back(new std::thread(&http::TcpServer::Connect, this,
                                            this->GetClient_(&this->socket_)));

  std::cout << "?" << std::endl;

  return;
}

void http::TcpServer::Connect(http::Client *_client) {
  size_t bytes = _client->Read();
  if (bytes < 0) {
    std::cerr << "Faild to read client message.\n"
              << strerror(errno) << std::endl;
    _client->Send(HTTP SERVERR END);
  } else {
    _client->buffer[bytes] = '\0';

    http::Request req = http::parse(std::string(_client->buffer));

    std::cout << "Request from " << this->name << std::endl;
    std::cout << "Method: " << req.method << "\nUri: " << req.uri
              << "\nFile: " << req.file << "\nArg: " << req.arg
              << "\nVersion: " << req.version
              << "\n\nHeader______\nHost: " << req.header.host
              << "\nagent: " << req.header.agent
              << "\naccept: " << req.header.accept
              << "\nlang: " << req.header.lang
              << "\nencoding: " << req.header.encoding
              << "\nconnection: " << req.header.connection
              << "\nupgrade: " << req.header.upgrade
              << "\ntype: " << req.header.type
              << "\nlength: " << req.header.length << "\n\n\n"
              << std::endl;
  }

  _client->Send(HTTP OK END);

  close(_client->socket);

  return;
}

void http::TcpServer::add(http::Method _method, const char *_path,
                          std::function<void(void *_pVoid)> _λ) {
  this->_vMap[_method][_path] = _λ;
  return;
}

/*
 */
