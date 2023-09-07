#ifndef CLIENT
#define CLIENT

#include <iostream>
#include <netinet/in.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <sys/socket.h>
#include <unistd.h>

#if DEBUG
#include <string>
#endif

namespace http {

class Client {
public:
  char buffer[4096];
  bool state;
  std::string type;

  Client(const int *_pSocket);
  virtual size_t Read();
  virtual int Send(const char *_pBuffer);
  ~Client();

protected:
  int socket_;
  sockaddr_in client_;
  socklen_t size_;
  size_t bufferSize_;
};

class SSLClient : public Client {
public:
#if DEBUG
  std::string Dtype = "ssl client";
#endif

  SSLClient(SSL_CTX *_pCTX, const int *_pSocket);
  size_t Read() override;
  int Send(const char *_pBuffer) override;
  ~SSLClient();

private:
  SSL *pSSL_;
};

} // namespace http

#endif // !CLIENT
