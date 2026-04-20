#pragma once

#include "../config.hpp"
#include "../logger/logger.hpp"
#include <concepts>
#include <functional>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

namespace protocol
{

template <typename T>
concept Serializable = requires(T t) {
  { std::string(t) } -> std::same_as<std::string>;
};

template <typename T>
concept Deserializable = requires(const std::string str) {
  { T(str) } -> std::same_as<T>;
};

template <typename P, typename T>
concept Sendable = requires(T t, int s, int f) {
  { P::template send<T>(s, t, f) } -> std::same_as<ssize_t>;
};

template <typename P, typename T>
concept Receivable = requires(T& t, int s, int f) {
  { P::template recv<T>(s, t, f) } -> std::same_as<ssize_t>;
};

} // namespace protocol

template <typename P>
concept Protocol = requires() {
  typename P::Response;
  typename P::Request;

  requires protocol::Serializable<typename P::Request>;
  requires protocol::Serializable<typename P::Response>;

  requires protocol::Deserializable<typename P::Request>;
  requires protocol::Deserializable<typename P::Response>;

  requires protocol::Sendable<P, typename P::Request>;
  requires protocol::Sendable<P, typename P::Response>;

  requires protocol::Receivable<P, typename P::Request>;
  requires protocol::Receivable<P, typename P::Response>;
};

namespace protocol
{

template <Protocol P> using Handler = std::function<typename P::Response(typename P::Request)>;

};

// template <Serializable T> ssize_t send(const int _socket, const T _t, const int _flag = 0)
// {
//   const std::string s(_t);
//   return ::send(_socket, s.c_str(), s.size(), _flag);
// }
//
// template <Protocol P, Deserializable T> ssize_t recv(const int _socket, T& _t, const int _flag = 0)
// {
//   auto log = Logger::New();
//
//   char buf[BUFFER_SIZE + 1]; // plus one for '\0'?
//   std::string s;
//   ssize_t bt;
//
//   bt = ::recv(_socket, buf, BUFFER_SIZE, _flag);
//   log.info("received bt = {}", bt);
//
//   if (bt <= 0) return bt; // client closed connection or recv failed
//
//   while (bt == BUFFER_SIZE)
//   {
//     s += buf;
//     bt = ::recv(_socket, buf, BUFFER_SIZE, _flag);
//     if (bt < 0) return bt; // recv failed
//   }
//
//   buf[bt] = '\0';
//   s += buf;
//   _t = T(s);
//
//   return bt;
// }
//
// // moue~
// template <HTTP P, typename T> ssize_t recv(const int _socket, T& _t, const int _flag = 0)
// {
//   auto log = Logger::New();
//
//   char buf[BUFFER_SIZE + 1]; // plus one for '\0'?
//   std::string s;
//   ssize_t bt;
//
// rcv:
//   bt = ::recv(_socket, buf, BUFFER_SIZE, _flag);
//   log.info("received bt = {}", bt);
//
//   if (bt <= 0) return bt; // client closed connection or recv failed
//
//   while (bt == BUFFER_SIZE)
//   {
//     s += buf;
//     bt = ::recv(_socket, buf, BUFFER_SIZE, _flag);
//     log.info("received bt = {}", bt);
//
//     if (bt < 0) return bt; // recv failed
//   }
//
//   buf[bt] = '\0';
//   s += buf;
//
//   try
//   {
//     _t = T(s);
//   }
//   catch (HTTP::ShortException)
//   {
//     goto rcv;
//   }
//
//   return bt;
// }
//
// } // namespace protocol
