#include "./tls-server.hpp"
#include <openssl/ssl.h>

http::TlsServer::TlsServer(const char *_pCertFile, const char *_pKeyFile)
    : http::TcpServer() {
  SSL_library_init();

  this->InitServerCTX_();
  this->LoadCertificates_(_pCertFile, _pKeyFile);
}

void http::TlsServer::InitServerCTX_() {
  OpenSSL_add_all_algorithms();
  SSL_load_error_strings();

  this->pCTX_ = SSL_CTX_new(TLS_method()); // TLSv1_2_server_method());

  if (this->pCTX_ == NULL) {
    ERR_print_errors_fp(stderr);
    abort();
  }

  return;
}

void http::TlsServer::LoadCertificates_(const char *_pCertFile,
                                        const char *_pKeyFile) {
  if (SSL_CTX_use_certificate_file(this->pCTX_, _pCertFile, SSL_FILETYPE_PEM) <=
      0) {
    ERR_print_errors_fp(stderr);
    abort();
  }

  if (SSL_CTX_use_PrivateKey_file(this->pCTX_, _pKeyFile, SSL_FILETYPE_PEM) <=
      0) {
    ERR_print_errors_fp(stderr);
    abort();
  }

  if (!SSL_CTX_check_private_key(this->pCTX_)) {
    std::cout << stderr << "Private key does not match the public certificate"
              << std::endl;
    abort();
  }

  return;
}

void http::TlsServer::ShowCerts() {
  X509 *cert;
  char *line;

  cert = SSL_get_peer_certificate(this->pSSL);
  std::cout << "2" << std::endl;
  /*
    if(cert != NULL){
      std::cout << "Server certificate:\n" << X509_NAME_oneline(const X509_NAME
  *a, char *buf, int size)
    }
  */
}

void http::TlsServer::ConnectionHandler_() {
  while (true) {
    Client client = Client(&this->socket_);
    this->pSSL = SSL_new(this->pCTX_);
    SSL_set_fd(pSSL, client.socket);
    std::cout << "0" << std::endl;

    int bytes;

    const char *ServerResponse =
        "<\Body><Name>aticleworld.com</Name><year>1.5</year><BlogType>Embedede "
        "and c\c++<\BlogType><Author>amlendra<Author><\Body>";

    const char *cpValidMessage =
        "<Body><UserName>aticle<UserName><Password>123<Password><\Body>";

    const char *er = "<Body><p>Hello World</p></Body>";

    if (SSL_accept(pSSL) == -1) {
      ERR_print_errors_fp(stderr);
      std::cout << "1" << std::endl;
    } else {
      this->ShowCerts();
      bytes = SSL_read(this->pSSL, client.buffer, sizeof(client.buffer));
      client.buffer[bytes] = '\0';
      std::cout << "3" << std::endl;

      std::cout << "Client message: " << client.buffer << std::endl;

      if (bytes > 0) {
        if (strcmp(cpValidMessage, client.buffer) == 0) {
          SSL_write(this->pSSL, ServerResponse, strlen(ServerResponse));
        } else {
          SSL_write(this->pSSL, "HTTP/1.1 400 Bad Request",
                    strlen("HTTP/1.1 400 Bad Request"));
          SSL_write(this->pSSL, er, strlen(er));
        }
      } else {
        ERR_print_errors_fp(stderr);
        std::cout << "4" << std::endl;
      }
    }
  }

  int sd = SSL_get_fd(this->pSSL);
  // SSL_free(ssl);
  close(sd);
}
