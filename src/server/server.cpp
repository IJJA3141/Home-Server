#include "server.hpp"
#include "../log.hpp"
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <unistd.h>

Tcp::Tcp(const Router *_router)
{
  if (_router == nullptr) {
    ERR("Tcp was initialized without a parser");
    exit(1);
  }

  this->router_ = _router;

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

  LOG("received call...");

  while (true) {
    this->active_client.push_back(new std::thread(&Tcp::connect, this, this->await_client()));
  };

  return;
}

Client *Tcp::await_client() { return new Client(this->socket_); }

Tcp::~Tcp()
{
  ::close(this->socket_);
  return;
}

Tls::Tls(const Router *_router, const std::string _cert_path, const std::string _key_path)
    : Tcp(_router)
{
  LOG("ssl initialization...");

  if (!SSLLIBINIT) {
    LOG("loading OpenSSL library...");
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    SSLLIBINIT = true;
  }

  this->ctx_ = SSL_CTX_new(TLS_method());
  if (this->ctx_ == NULL) {
    VERBERR("loading SSL method failed.");
    exit(1);
  }

  LOG("loading certificate...");
  if (SSL_CTX_use_certificate_file(this->ctx_, _cert_path.c_str(), SSL_FILETYPE_PEM) <= 0) {
    VERBERR("loading certificate file failed.");
    ERR_print_errors_fp(stderr);
    exit(1);
  }

  if (SSL_CTX_use_PrivateKey_file(this->ctx_, _key_path.c_str(), SSL_FILETYPE_PEM) <= 0) {
    VERBERR("loading certificate key failed.");
    ERR_print_errors_fp(stderr);
    exit(-1);
  }

  if (!SSL_CTX_check_private_key(this->ctx_)) {
    VERBERR("private key does not match the pulic certificate.");
    ERR_print_errors_fp(stderr);
    exit(1);
  }

  return;
}

Client *Tls::await_client() { return new SSLClient(this->socket_, this->ctx_); }

Tls::~Tls()
{
  OPENSSL_cleanup();
  return;
}

//
void Tcp::connect(Client *_client)
{
  Request req = _client->read();

  Response res = this->router_->respond(req);
  _client->send(res);

  delete _client;

  return;
}
