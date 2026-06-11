#include "../config.hpp"
#include "../ipc/transport/transport.hpp"
#include "../utils/base64.hpp"
#include "../utils/utils.hpp"
#include "auth.hpp"
#include <expected>
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <optional>
#include <variant>

using ATP = protocol::ATP;
using Response = protocol::ATP::Response;
using Request = protocol::ATP::Request;

ATP::VAL::Response handle_val(std::vector<auth::User> _map, ATP::VAL::Request _request)
{
  std::optional<auth::User&> user = std::nullopt;

  for (auto& _ : _map)
    if (_.set.contains(_request.id))
    {
      user = _;
      break;
    }

  if (user)
  {
    if (user->set[_request.id].user_agent != _request.user_agent)
    {
      return {std::unexpected("non matching user agent")};
    }
    else
    {
      return {user->uuid};
    }
  }
  else
  {
    return {std::unexpected("id not found")};
  }
}

ATP::LOG::Response handle_log(std::vector<auth::User> _users, ATP::LOG::Request _request)
{
  std::optional<auth::User&> user = std::nullopt;

  for (auto& _ : _users)
    if (_.name == _request.user)
    {
      user = _;
      break;
    }

  if (user)
  {
    // unbase64 _request.pwd
    // std::optional<std::string> pwd = safe::base64_decode(_request.pwd);
    // if (!pwd) return {std::unexpected("failed to unbase64 the password")};

    // hash _request.pwd
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit(ctx, EVP_sha256());
    EVP_DigestUpdate(ctx, _request.pwd.c_str(), _request.pwd.size());

    unsigned char hash[EVP_MD_size(EVP_sha256())];
    EVP_DigestFinal_ex(ctx, hash, nullptr);
    EVP_MD_CTX_free(ctx);
    // compare

    if (memcmp(user->pwd.begin(), hash, sizeof hash))
    {
      return {std::unexpected("bad password")};
    }

    return {user->uuid};
  }
  else
  {
    return {std::unexpected("user not found")};
  }
}

ATP::GEN::Response handle_gen(ATP::GEN::Request _request) { return {}; }

int main(void)
{
  std::string pwd = "password";
  EVP_MD_CTX* ctx = EVP_MD_CTX_new();
  EVP_DigestInit(ctx, EVP_sha256());
  EVP_DigestUpdate(ctx, pwd.c_str(), pwd.size());

  std::array<unsigned char, 256> hash;
  EVP_DigestFinal_ex(ctx, hash.begin(), nullptr);
  EVP_MD_CTX_free(ctx);

  std::vector<auth::User> map{
      auth::User{Uuid::parse("669db884-1342-4d97-b87a-93b71bb0100a"),
                 {{Uuid::parse("9c83dc0a-0531-4080-b90c-76b41e06a252"),
                   {"", "Mozilla/5.0 (X11; Linux x86_64; rv:152.0) Gecko/20100101 Firefox/152.0"}}},
                 "test",
                 hash}};

  const auto handlers = overloads{
      [&](ATP::VAL::Request _request) -> Response { return Response{ATP::Type::VAL, handle_val(map, _request)}; },
      [&](ATP::LOG::Request _request) -> Response { return Response{ATP::Type::LOG, handle_log(map, _request)}; },
      [](ATP::GEN::Request _request) -> Response { return Response{ATP::Type::GEN, handle_gen(_request)}; },
  };

  ipc::TransportServer<protocol::ATP> server(
      AUTH_IP, AUTH_PORT,
      [&](Request _request) -> Response { return std::visit<Response>(handlers, _request.request); }, {});
  server.listen();

  return 0;
}
