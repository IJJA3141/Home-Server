#pragma once

#include "client.hpp"
#include "router.hpp"

#include <filesystem>
#include <set>

/**
 * @class Tcp
 * @brief Represents a basic TCP server.
 *
 * Handles TCP socket creation, binding, and event-driven client management.
 * Uses epoll for scalable I/O multiplexing and manages connected clients.
 */
class Tcp
{
public:
  /**
   * @brief Constructs a TCP server.
   *
   * Sets up the server to listen on the specified port and associates it with a router
   * for handling incoming requests.
   *
   * @param _port The port number the server will listen on.
   * @param _router Reference to a Router object to manage request routing.
   */
  Tcp(int _port, const Router& _router);

  /**
   * @brief Destructs the TCP server.
   * Closes the server's socket.
   */
  ~Tcp();

  /**
   * @brief Checks if the server is currently running.
   * @return True if the server is running, false otherwise.
   */
  inline bool is_running() const { return running_; }

  /**
   * @brief Starts the server's listen loop.
   *
   * Binds the socket, begins accepting incoming connections,
   * and holds the thread while the server is listenning.
   */
  void listen();

  /**
   * @brief Closes the server and all active connections.
   */
  void close();

protected:
  sockaddr_in addr_;
  int socket_;
  int epoll_;

private:
  std::set<const Client*> client_bay_;
  const Router router_;
  bool running_;

  void server_event(const bool _closed);
  void client_event(const bool _closed, const Client* _client);

  virtual Client* anchor_client() const;
};

/**
 * @class Tls
 * @brief Represents a TLS-enabled TCP server.
 *
 * Extends the Tcp server class to provide secure communication using TLS.
 * Manages SSL context initialization and handles TLS clients.
 */
class Tls : public Tcp
{
public:
  /**
   * @brief Constructs a TLS server.
   *
   * Initializes the TLS server with the given port, certificate, private key,
   * and routing configuration.
   *
   * @param _port The port number to listen on.
   * @param _cert Path to the TLS certificate file.
   * @param _key Path to the private key file.
   * @param _router Reference to the request router.
   */
  Tls(const int _port, const std::filesystem::path& _cert, const std::filesystem::path& _key,
      const Router& _router);

  /**
   * @brief Destructs the TLS server.
   * Closes the server's socket and if it's the last instance of a TLS server cleans up the SSL_LIB.
   */
  ~Tls();

private:
  static bool SSL_LIB_INIT_;
  static size_t ATOMIC_COUNT_;

  SSL_CTX* ctx_;

  Client* anchor_client() const override;
};
