#pragma once

#include "../../common/uuid.hpp"
#include "../protocol.hpp"
#include <cstddef>
#include <cstdint>
#include <netinet/in.h>
#include <span>
#include <variant>

namespace protocol
{

struct ipcp
{
  struct snd
  {
    Uuid client_id;
    uint64_t length; // in big endian
    std::span<const std::byte> body;

    snd(Uuid id, std::span<const std::byte> body) : client_id{id}, length{htobe64(body.size())}, body{body} {}

    static constexpr int HEADER_SIZE = Uuid::PARSED_SIZE + sizeof length;
  };

  struct del
  {
    Uuid client_id;
    del(Uuid uuid) : client_id{uuid} {};
    del(const del&) = default;
  };

  struct Request
  {
    enum class Type : char
    {
      ADD,
      SND,
      DEL,
    };

    struct add
    {
      Uuid client_id;
      in_port_t client_port; // in network byte order
      in_addr_t client_addr; // in network byte order
      ConnectionType over;
    };

    using snd = snd;
    using del = del;

    size_t insert(std::span<std::byte> dst);
    std::variant<add, snd, del> request;

    // Request(std::variant<add, snd, del> _request);

    class ParserContext;
  };

  struct Response
  {
    enum class Type : char
    {
      SND,
      DEL,
    };

    using snd = snd;
    using del = del;

    std::variant<snd, del> request;

    class ParserContext;

    static size_t parse(std::span<const char>, ParserContext&);
  };
};

class ipcp::Response::ParserContext
{
public:
  ParserResult result;

  ParserContext();

  Response construct();
  void reset();

private:
  enum
  {
    UUID,
    LENGTH,
  } state_;

  Uuid uuid_;
  uint64_t length_;
  size_t already_parsed_;
};

} // namespace protocol
