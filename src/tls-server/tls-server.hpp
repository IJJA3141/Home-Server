#ifndef TLS_SERVER
#define TLS_SERVER

#include "../tcp-server/tcp-server.hpp"

#include <openssl/err.h>
#include <openssl/ssl.h>

namespace http {

class TlsServer : public TcpServer {
public:
  TlsServer(const char *_pCertFile, const char *_pKeyFile);

private:
  SSL_CTX *pCTX_;

  void InitServerCTX_();
  void LoadCertificates_(const char *_pCertFile, const char *_pKeyFile);
  inline Client GetClient_(const int *_pSocket) override {
    return http::SSLClient(this->pCTX_, _pSocket);
  };
};

} // namespace http

#endif // !TLS_SERVER
