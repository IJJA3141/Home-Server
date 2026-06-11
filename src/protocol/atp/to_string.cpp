#include "atp.hpp"
#include <format>

protocol::ATP::VAL::Request::operator std::string() const
{
  return std::format("{}\n{}\n{}\n{}\n", source, (std::string)id, ip, user_agent);
}

protocol::ATP::VAL::Response::operator std::string() const
{
  return std::format("{}\n", uuid.has_value() ? uuid.value() : uuid.error());
}

protocol::ATP::LOG::Request::operator std::string() const
{
  return std::format("{}\n{}\n{}\n{}\n", user, pwd, ip, user_agent);
}

protocol::ATP::LOG::Response::operator std::string() const
{
  return std::format("{}\n", id.has_value() ? id.value() : id.error());
}

protocol::ATP::GEN::Request::operator std::string() const
{
  return std::format("{}\n{}\n{}\n{}\n", source, (std::string)id, ip, user_agent);
}

protocol::ATP::GEN::Response::operator std::string() const
{
  return std::format("{}\n", uuid.has_value() ? uuid.value() : uuid.error());
}
