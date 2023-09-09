#include "./tcp-server.hpp"
#include <charconv>
#include <unistd.h>

int http::TcpServer::serverCount_ = 0;

http::TcpServer::TcpServer(const char *_name) {
  std::cout << "\n_____________________\nServer initialization" << std::endl;

  if (_name == nullptr) {
    this->name = "Server " + std::to_string(http::TcpServer::serverCount_++);
  } else
    this->name = _name;

  std::cout << "Creating server socket..." << std::endl;

  this->socket_ = socket(AF_INET, SOCK_STREAM, 0);
  if (this->socket_ == -1) {
    std::cerr << "Socket creation failed.\n" << strerror(errno) << std::endl;
    exit(120);
  };

  std::cout << "Setting server options..." << std::endl;
  int opt = 1;
  if (setsockopt(this->socket_, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt))) {
    std::cerr << "Setting options failed.\n" << strerror(errno) << std::endl;
    exit(121);
  }

  return;
}

void http::TcpServer::Bind(const char *_pPort) {
  std::cout << "Biding socket to sockaddr on port: " << _pPort << "..."
            << std::endl;
  this->hint_.sin_family = AF_INET;
  this->hint_.sin_addr.s_addr = htonl(INADDR_ANY);
  this->hint_.sin_port = htons(std::atoi(_pPort));
  if (::bind(this->socket_, (struct sockaddr *)&this->hint_,
             sizeof(this->hint_)) == -1) {
    std::cerr << "IP/PORT binding failed.\n" << strerror(errno) << std::endl;
    exit(122);
  }

  return;
}

void http::TcpServer::Listen() {
  std::cout << "Mark socket for listening..." << std::endl;
  if (::listen(this->socket_, SOMAXCONN) == -1) {
    std::cerr << "Listening failed.\n" << strerror(errno) << std::endl;
    exit(123);
  }

  std::cout << "\nReceived call..." << std::endl;

  while (true)
    this->vThread.push_back(new std::thread(&http::TcpServer::Connect, this,
                                            this->GetClient_(&this->socket_)));

  return;
}

void http::TcpServer::Connect(http::Client *_pClient) {
  size_t bytes = _pClient->Read();

  if (bytes < 0) {
    std::cerr << "Faild to read client message.\n"
              << strerror(errno) << std::endl;
    _pClient->Send(HTTP SERVERR END);
  } else {
    _pClient->buffer[bytes] = '\0';

    std::cout << "New request from " << this->name << std::endl;
    http::Request req = http::Parse(std::string(_pClient->buffer));
  }

  const char *location = "Location https://www.google.com\r\n"; // site
  const char *ok = "HTTP/1.1 200 OK\r\n";
  const char *res =
      "<html><head><link rel='stylesheet' type='text/css' "
      "href=https://ijja.dev/static/style.css'></head><body><p>Hellow "
      "World!\nFrom Debian Server.</p></body></html>";
  int resSize = strlen(res);

  char clbuf[32];
  sprintf(clbuf, "Content-length: %ld\r\n", (long)resSize);

  _pClient->Send(ok);
  _pClient->Send(clbuf);
  _pClient->Send("Content-Type: text/html\r\n");
  _pClient->Send("\r\n");
  _pClient->Send(res);

  delete _pClient;

  return;
}

void http::TcpServer::Add(http::Method _method, const char *_path,
                          std::function<void(void *_pVoid)> _λ) {
  this->_vMap[_method][_path] = _λ;
  return;
}

http::TcpServer::~TcpServer() {
  std::cout << "Closing " << this->name << std::endl;
  close(this->socket_);

  return;
}
