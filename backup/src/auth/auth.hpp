#pragma once

#include "../ipc/transport/transport.hpp"
#include "../protocol/http/http.hpp"
#include "../utils/uuid.hpp"
#include <openssl/evp.h>

namespace auth
{

protocol::Middleware<protocol::HTTP> create_auth_middleware(std::string login, std::string source,
                                                            ipc::TransportClient<protocol::ATP>& ipc_client);

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
