#include "./client.hpp"
#include <iostream>
#include <sys/socket.h>

http::Client::Client(const int *_pSocket) {
  this->type = "non ssl client";

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
