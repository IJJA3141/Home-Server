#include "./tls-server.hpp"

http::TlsServer::TlsServer(const char *_pCertFile, const char *_pKeyFile)
    : http::TcpServer() {
  this->InitServerCTX_();
  this->LoadCertificates_(_pCertFile, _pKeyFile);
}

void http::TlsServer::InitServerCTX_() {
  std::cout << "Loading OpenSSL library..." << std::endl;
  SSL_library_init();
  OpenSSL_add_all_algorithms();
  SSL_load_error_strings();

  this->pCTX_ = SSL_CTX_new(TLS_method()); // TLSv1_2_server_method());

  if (this->pCTX_ == NULL) {
    std::cerr << "Loading SSL method failed.\n" << stderr << std::endl;
    abort();
  }

  return;
}

void http::TlsServer::LoadCertificates_(const char *_pCertFile,
                                        const char *_pKeyFile) {
  std::cout << "Loading certificate..." << std::endl;
  if (SSL_CTX_use_certificate_file(this->pCTX_, _pCertFile, SSL_FILETYPE_PEM) <=
      0) {
    std::cerr << "Loading certificate file failed.\n" << stderr << std::endl;
    abort();
  }

  if (SSL_CTX_use_PrivateKey_file(this->pCTX_, _pKeyFile, SSL_FILETYPE_PEM) <=
      0) {
    std::cerr << "Loading certificate key failed.\n" << stderr << std::endl;
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

void http::TlsServer::ConnectionHandler_() {
  while (true) {
    Client client = Client(&this->socket_);
    this->pSSL = SSL_new(this->pCTX_);
    SSL_set_fd(pSSL, client.socket);

    int bytes;

    const char *location = "Location https://www.google.com\r\n"; // site
    const char *ok = "HTTP/1.1 200 OK\r\n";
    const char *res =
        "<html><body><p>Hellow World!\nFrom Debian Server.</p></body></html>";
    int resSize = strlen(res);

    char clbuf[32];
    sprintf(clbuf, "Content-length: %ld\r\n", (long)resSize);

    if (SSL_accept(pSSL) == -1) {
      send(client.socket, ok, strlen(ok), 0);
      send(client.socket, location, strlen(location), 0);
      ERR_print_errors_fp(stderr);
    } else {
      //   this->ShowCerts();
      bytes = SSL_read(this->pSSL, client.buffer, sizeof(client.buffer));
      client.buffer[bytes] = '\0';

      std::cout << "Client message: " << client.buffer << std::endl;

      if (bytes > 0) {
        // if (strcmp(cpValidMessage, client.buffer) == 0) {
        // SSL_write(this->pSSL, "HTTP/1.1 400 Bad Request",strlen("HTTP/1.1 400
        // Bad Request"));
        SSL_write(this->pSSL, ok, strlen(ok));
        SSL_write(this->pSSL, clbuf, strlen(clbuf));
        SSL_write(this->pSSL, "Content-Type: text/html\r\n",
                  strlen("Content-Type: text/html\r\n"));
        SSL_write(this->pSSL, "\r\n", strlen("\r\n"));
        SSL_write(this->pSSL, res, resSize);
      } else {
        ERR_print_errors_fp(stderr);
      }
    }
  }

  int sd = SSL_get_fd(this->pSSL);
  // SSL_free(ssl);
  close(sd);
}
