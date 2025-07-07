#pragma once

#include "http.hpp"

#include <netinet/in.h>
#include <openssl/crypto.h>

/**
 * @class Client
 * @brief Represents a nonblocking network client managed via epoll.
 *
 * This class encapsulates the behavior of a connected client socket, including
 * registration with an epoll instance, reading HTTP requests, and writing responses.
 */
class Client
{
public:
  /**
   * Tacitly const
   * Set to true if the client setup and registration with epoll completed successfully.
   */
  bool moored;

  /**
   * @brief Constructs a Client.
   *
   * Sets up the client connection and registers the client socket with the specified epoll instance.
   *
   * @param _epoll The epoll file descriptor where the client socket will be registered.
   * @param _socket The socket file descriptor for the client connection.
   * @param _addr The socket address of the client.
   */
  Client(const int _epoll, const int _socket, const sockaddr_in _addr);

  /**
   * @brief Destructs the Client.
   * Closes the client socket and deregisters the client from the epoll instance.
   */
  ~Client();

  /**
   * @brief Reads and parses an HTTP request from the client.
   * @return An http::Request object containing the client's request.
   */
  http::Request read() const;

  /**
   * @brief Sends an HTTP response to the client.
   * @param _response The HTTP response to send.
   */
  void write(const http::Response _response) const;

protected:
  int socket_;

  void close() const;

private:
  const int epoll_;

  virtual ssize_t recv(char* const _buffer) const;
  virtual int send(const std::string& _msg) const;
};

/**
 * @class SSL_Client
 * @brief Represents an SSL nonblocking network client.
 *
 * Inherits from the Client class and extends it to support secure communication
 * using OpenSSL. Overrides the data transmission methods to use SSL-specific APIs.
 */
class SSL_Client : public Client
{
public:
  /**
   * @brief Constructs an SSL_Client.
   *
   * Initializes an SSL-enabled nonblocking client by setting up the SSL context,
   * associating the socket with the epoll instance, and preparing
   * the SSL structure for encrypted communication.
   *
   * @param _epoll The epoll file descriptor where the client socket will be registered.
   * @param _socket The socket file descriptor for the client connection.
   * @param _addr The socket address of the client.
   * @param _ctx The SSL context used to create the SSL connection.
   */
  SSL_Client(const int _epoll, const int _socket, const sockaddr_in _addr, SSL_CTX* const _ctx);

  /**
   * @brief Destructs the SSL_Client.
   *
   * Cleans up SSL resources, closes the client socket, and deregisters
   * the client from the epoll instance.
   */
  ~SSL_Client();

private:
  ::SSL* ssl_;

  bool dehaling() const;

  ssize_t recv(char* const _buffer) const override;
  int send(const std::string& _msg) const override;
};
