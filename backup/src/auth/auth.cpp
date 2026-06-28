#include "auth.hpp"
#include "../logger/logger.hpp"

using HTTP = protocol::HTTP;
using ATP = protocol::ATP;

protocol::Middleware<HTTP> auth::create_auth_middleware(std::string login, std::string source,
                                                  ipc::TransportClient<ATP>& ipc_client)
{
  return [=, &ipc_client](HTTP::Request& request, protocol::Handler<HTTP> callback) -> HTTP::Response {
    auto log = Logger::get("auth middleware");
    // TODO might want to check for https connection

    std::map<std::string, std::string> cookies = HTTP::parse_cookies(request.headers["cookie"]);
    if (cookies.contains(HTTP_SESSION_ID))
    {
      auto session_id = Uuid::parse_safe(cookies[HTTP_SESSION_ID]);
      if (session_id)
      {
        log.debug("sending request to auth agent for {} validation", request.headers["cookie"]);
        auto res = ipc_client.transmit(
            {ATP::Type::VAL, ATP::VAL::Request{source, session_id.value(), request.headers[HTTP_CLIENT_IP],
                                               request.headers["user-agent"]}});

        if (res.type != ATP::Type::VAL)
        {
          log.error("invalid ipc response type");
          return HTTP::standard_response(500);
        }

        auto uuid = std::get<ATP::VAL::Response>(res.response).uuid;
        if (uuid)
        {
          log.debug("user (IP={}) identified as (uuid={})", request.headers[HTTP_CLIENT_IP],
                    std::string(uuid.value()));

          request.headers[HTTP_UUID] = uuid.value();
          return callback(request);
        }
      }
    }

    log.debug("auth failed redirecting user to login page {}", login);
    return {protocol::HTTP::Version::HTTP_11,
            303,
            "See Other",
            {{"location", login + "?returnto=" + request.path}, {"content-length", "0"}}};
  };
}
