#include "./client.hpp"

http::Client::Client(const int *_pSocket) {
  this->type = "Non ssl client";
  this->size_ = sizeof(this->client_);
  this->bufferSize_ = sizeof(this->buffer);
  this->socket_ =
      accept(*_pSocket, (struct sockaddr *)&this->client_, &this->size_);

  if (this->socket_ == -1) {
    std::cerr << "Connection with the client failed.\n"
              << strerror(errno) << std::endl;
    this->state = false;
  }
#if DEBUG
  std::cout << "New " << this->type << " connected";
#endif

  this->state = true;
  return;
}

size_t http::Client::Read() {
  return ::read(this->socket_, this->buffer, this->bufferSize_);
};

int http::Client::Send(const char *_pBuffer) {
  return ::write(this->socket_, _pBuffer, strlen(_pBuffer));
};

http::Client::~Client() {
#if DEBUG
  std::cout << this->type << " closed" << std::endl;
#endif
  ::close(this->socket_);
  return;
}
