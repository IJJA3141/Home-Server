#ifndef TLS_SERVER
#define TLS_SERVER

#include "../tcp-server/tcp-server.hpp"

#include <arpa/inet.h>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <malloc.h>
#include <netinet/in.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/ssl.h>
#include <openssl/types.h>
#include <openssl/x509.h>
#include <resolv.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

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
};

} // namespace http

#endif // !TLS_SERVER
