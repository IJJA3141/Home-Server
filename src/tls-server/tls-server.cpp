#include "./tls-server.hpp"

http::TlsServer::TlsServer(const char *_pCertFile, const char *_pKeyFile,
                           const char *_name)
    : http::TcpServer(_name) {
  std::cout << "\n_______________________\nInitializing SSL server"
            << std::endl;

  this->InitServerCTX_();
  this->LoadCertificates_(_pCertFile, _pKeyFile);

  return;
}

void http::TlsServer::InitServerCTX_() {
  std::cout << "Loading OpenSSL library..." << std::endl;

  SSL_library_init();
  OpenSSL_add_all_algorithms();
  SSL_load_error_strings();

  this->pCTX_ = SSL_CTX_new(TLS_method()); // TLSv1_2_server_method());

  if (this->pCTX_ == NULL) {
    std::cerr << "Loading SSL method failed.\n" << stderr << std::endl;
    exit(130);
  }

  return;
}

void http::TlsServer::LoadCertificates_(const char *_pCertFile,
                                        const char *_pKeyFile) {
  std::cout << "Loading certificate..." << std::endl;

  if (SSL_CTX_use_certificate_file(this->pCTX_, _pCertFile, SSL_FILETYPE_PEM) <=
      0) {
    std::cerr << "Loading certificate file failed." << std::endl;
    ERR_print_errors_fp(stderr);
    exit(131);
  }

  if (SSL_CTX_use_PrivateKey_file(this->pCTX_, _pKeyFile, SSL_FILETYPE_PEM) <=
      0) {
    std::cerr << "Loading certificate key failed." << std::endl;
    ERR_print_errors_fp(stderr);
    exit(132);
  }

  if (!SSL_CTX_check_private_key(this->pCTX_)) {
    std::cerr << "Private key does not match the public certificate."
              << std::endl;
    ERR_print_errors_fp(stderr);
    exit(133);
  }

  return;
}

http::TlsServer::~TlsServer() {
  OPENSSL_cleanup();
  return;
}
