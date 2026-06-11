#pragma once

#include "../ipc/transport/transport.hpp"
#include "../logger/logger.hpp"
#include "../protocol/http/http.hpp"
#include "../utils/uuid.hpp"
#include <openssl/evp.h>
#include <stdexcept>

namespace auth
{

// protocol::Middleware<protocol::HTTP> create_auth_middleware(const std::string& login, const std::string& source,
//                                                             ipc::TransportClient<protocol::ATP>& ipc_client);

using HTTP = protocol::HTTP;
using ATP = protocol::ATP;

inline protocol::Middleware<HTTP> create_auth_middleware(const std::string login, const std::string source,
                                                         ipc::TransportClient<ATP>& ipc_client)
{
  return [=, &ipc_client](HTTP::Request& _request, protocol::Handler<HTTP> _callback) -> HTTP::Response {
    auto log = Logger::get("auth middleware");
    // if (_request.headers[HTTP_CONNECTION_TYPE] != CONNECTION_TYPE_HTTPS)
    //   return {HTTP::Version::HTTP_11,
    //           308,
    //           "Permanent Redirect",
    //           {{"location", "https://" + _request.headers["host"] + _request.path}}};

    std::map<std::string, std::string> cookies = HTTP::parse_cookies(_request.headers["cookie"]);
    std::optional<Uuid> uuid;

    if (cookies.contains(HTTP_SESSION_ID) && (uuid = Uuid::parse_safe(cookies[HTTP_SESSION_ID])))
    {
      log.debug("sending request to auth agent for (uuid={}) validation", _request.headers["cookie"]);
      auto res = ipc_client.transmit(
          {ATP::Type::VAL, ATP::VAL::Request{source, uuid.value(), _request.headers[HTTP_CLIENT_IP],
                                             _request.headers["user-agent"]}});

      if (res.type != ATP::Type::VAL)
      {
        log.crit("invalid ipc response type");
        throw std::runtime_error("invalid ipc response type");
      }

      auto r = std::get<ATP::VAL::Response>(res.response);

      if (r.uuid)
      {
        log.debug("user (IP={}) identified as (uuid={})", _request.headers[HTTP_CLIENT_IP],
                  std::string(r.uuid.value()));

        _request.headers[HTTP_UUID] = r.uuid.value();
        return _callback(_request);
      }
    }

    log.debug("redirecting user to login page {}", login);
    return {
        protocol::HTTP::Version::HTTP_11, 303, "See Other", {{"location", login + "?returnto=" + _request.path}}};
  };
}

struct ctx
{
  std::string ip;
  std::string user_agent;
};

struct User
{
  Uuid uuid;
  std::map<Uuid, ctx> set;
  std::string name;
  std::array<unsigned char, 256> pwd;

  // some way to get host permissions
};

} // namespace auth
