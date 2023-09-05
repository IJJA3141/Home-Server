#ifndef CLIENT
#define CLIENT

#include <iostream>
#include <netinet/in.h>
#include <openssl/ssl.h>
#include <unistd.h>

namespace http {

struct Client {
public:
  char buffer[4096];
  int socket;
  const char *type;

  Client(const int *_pSocket);
  virtual size_t Read();
  virtual int Send(const char *_pBuffer);

protected:
  sockaddr_in client;
  socklen_t size;
  size_t bufferSize_;
};

class SSLClient : public Client {
public:
  SSLClient(SSL_CTX *_pCTX, const int *_pSocket);
  size_t Read() override;
  int Send(const char *_pBuffer) override;

private:
  SSL *pSSL_;
};

} // namespace http
#endif // !CLIENT
