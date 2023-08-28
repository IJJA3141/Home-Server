#ifndef CLIENT
#define CLIENT

#include <netinet/in.h>
#include <openssl/ssl.h>
#include <unistd.h>

namespace http {

struct Client {
public:
  char buffer[4096];
  int socket;

  Client(const int *_pSocket);
  inline virtual size_t Read() {
    return ::read(this->socket, this->buffer, this->bufferSize_);
  };
  inline virtual int Send(const char *_pBuffer) {
    return ::write(this->socket, _pBuffer, strlen(_pBuffer));
  };

protected:
  sockaddr_in client;
  socklen_t size;
  size_t bufferSize_;
};

struct SSLClient : public Client {
public:
  SSLClient(SSL_CTX *_pCTX, const int *_pSocket);
  inline size_t Read() override {
    return SSL_read(this->pSSL_, this->buffer, this->bufferSize_);
  };
  int Send(const char *_pBuffer) override {
    return SSL_write(this->pSSL_, _pBuffer, strlen(_pBuffer));
  };

private:
  SSL *pSSL_;
};

} // namespace http
#endif // !CLIENT
