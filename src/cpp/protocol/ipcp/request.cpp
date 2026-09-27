#include "../../common/utils.hpp"
#include "ipcp.hpp"
#include <arpa/inet.h>
#include <cstddef>
#include <cstring>
#include <unistd.h>

using namespace protocol;
using Request = ipcp::Request;

size_t add(std::span<std::byte> _dst, Request::add _req)
{
  if (_dst.size() < sizeof(Request::Type) + sizeof _req) throw std::invalid_argument("req. didn't fit in dst.");

  _dst[0] = (std::byte)Request::Type::ADD;
  memcpy(_dst.data() + sizeof(Request::Type), &_req, sizeof _req);

  return sizeof _req;
};

size_t snd(std::span<std::byte> _dst, Request::snd _req)
{
  auto body = _req.body;
  auto dst = _dst.data();

  if (_dst.size() < sizeof(Request::Type) + Request::snd::HEADER_SIZE + body.size())
    throw std::invalid_argument("req. didn't fit in dst.");

  _dst[0] = (std::byte)Request::Type::SND;
  memcpy(dst + sizeof(Request::Type), &_req, Request::snd::HEADER_SIZE);
  memcpy(dst + sizeof(Request::Type) + Request::snd::HEADER_SIZE, body.data(), body.size());

  return Request::snd::HEADER_SIZE + body.size();
};

size_t del(std::span<std::byte> _dst, Request::del _req)
{
  if (_dst.size() < sizeof(Request::Type) + Uuid::PARSED_SIZE)
    throw std::invalid_argument("req. didn't fit in dst.");

  _dst[0] = (std::byte)Request::Type::DEL;
  memcpy(_dst.data() + sizeof(Request::Type), _req.client_id.begin(), Uuid::PARSED_SIZE);

  return Uuid::PARSED_SIZE;
};

size_t ipcp::Request::insert(std::span<std::byte> dst)
{ // clang-format off
  return sizeof(Request::Type) + std::visit(overloads{
    std::bind_front(::add, dst),
    std::bind_front(::snd, dst),
    std::bind_front(::del, dst)
  }, this->request);
  // clang-format on
}
