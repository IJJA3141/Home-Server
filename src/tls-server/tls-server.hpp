#ifndef TLS_SERVER
#define TLS_SERVER

#include "../tcp-server/tcp-server.hpp"
#include "../parser/parser.hpp"

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <string>

namespace http {

class TlsServer : public TcpServer {
public:
  TlsServer(const char *_pCertFile, const char *_pKeyFile);

private:
  SSL_CTX *pCTX_;
  SSL *pSSL;

  void InitServerCTX_();
  void LoadCertificates_(const char *_pCertFile, const char *_pKeyFile);
  void ConnectionHandler_() override;

  inline http::Client GetClient_(const int *_pSocket) override {
    return http::SSLClient(this->pCTX_, _pSocket);
  };
};

} // namespace http

#endif // !TLS_SERVER
