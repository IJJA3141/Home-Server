#pragma once

#include "../ipc/transport/transport.hpp"
#include "../protocol/http/http.hpp"
#include "../utils/uuid.hpp"

namespace auth
{

inline protocol::Middleware<protocol::HTTP> authoritative_client()
{
  ipc::TransportClient<protocol::ATP::VAL> ipc_client("", 0);

  return [&ipc_client](protocol::HTTP::Request& _request,
                       protocol::Handler<protocol::HTTP> _callback) -> protocol::HTTP::Response {
    protocol::HTTP::Request request{protocol::HTTP::Method::GET, "", protocol::HTTP::Version::HTTP_09, {}, ""};
    return _callback(_request);
  };
}

struct User
{
  Uuid uuid;
  std::string name;
  std::string pwd;

  // some way to get host permissions
};

} // namespace auth
