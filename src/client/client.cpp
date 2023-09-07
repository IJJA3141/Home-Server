#include "./client.hpp"

http::Client::Client(const int *_pSocket) {

  this->size = sizeof(this->client);
  this->bufferSize_ = sizeof(this->buffer);

  this->socket =
      accept(*_pSocket, (struct sockaddr *)&this->client, &this->size);

  if (this->socket == -1) {
    std::cerr << "Connection with the client failed.\n"
              << strerror(errno) << std::endl;
    abort();
  }

  return;
}

size_t http::Client::Read() {
  std::cout << "\n\n\n FUCK \n\n\n" << std::endl;
  return ::read(this->socket, this->buffer, this->bufferSize_);
};

int http::Client::Send(const char *_pBuffer) {
  return ::write(this->socket, _pBuffer, strlen(_pBuffer));
};
