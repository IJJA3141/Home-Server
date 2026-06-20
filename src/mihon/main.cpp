#include "../auth/auth.hpp"
#include "../config.hpp"
#include "../ipc/transport/transport.hpp"
#include "../logger/logger.hpp"
#include "../routing/routing.hpp"
#include "../utils/utils.hpp"
#include <filesystem>
#include <format>
#include <netinet/tcp.h>

using Method = protocol::HTTP::Method;
using Version = protocol::HTTP::Version;
using Response = protocol::HTTP::Response;
using Request = protocol::HTTP::Request;
using Handler = protocol::Handler<protocol::HTTP>;

bool safe_path(std::string_view str) { return true; }

int main(void)
{
  ipc::TransportClient<protocol::ATP> auth_client(AUTH_IP, AUTH_PORT);
  auth_client.connect();

  Router router;
  router.add(Method::GET, "/static/*", [](protocol::HTTP::Request& request) -> Response {
    std::string path = "data/static/" + request.headers[HTTP_WILDCARD];
    if (safe_path(path) && std::filesystem::exists(path))
    {
      auto file = load_file(path);
      return {Version::HTTP_11,
              200,
              "OK",
              {{"content-type", "text/html"}, {"content-length", std::format("{}", file.size())}},
              file};
    }
    else
    {
      auto file = load_file("data/html/not_found.html");
      return {Version::HTTP_11,
              404,
              "Not Found",
              {{"content-type", "text/html"}, {"content-length", std::format("{}", file.size())}},
              file};
    }
  });

  router.add(Method::GET, "/", [](Request&) -> Response {
    auto file = load_file("data/html/index.html");
    return {Version::HTTP_11,
            200,
            "OK",
            {{"content-type", "text/html"}, {"content-length", std::format("{}", file.size())}},
            file};
  });

  auto auth_middleware = auth::create_auth_middleware("/login", "mihon", auth_client);

  router.add(Method::GET, "/login_test", auth_middleware, [](Request& request) -> Response {
    auto uuid = "uuid := " + request.headers[HTTP_UUID];
    auto session_id = "session id := " + request.headers[HTTP_SESSION_ID];
    Logger::debug("success full login {{uuid={}}} {{session-id={}}}", uuid, session_id);

    auto file = load_file("data/html/login_test.html");
    file = std::vformat(file, std::make_format_args(uuid, session_id));

    return {Version::HTTP_11,
            200,
            "Ok",
            {{"content-type", "text/html"}, {"content-length", std::format("{}", file.size())}},
            file};
  });

  router.add(Method::GET, "/login", [](Request& request) -> Response {
    auto querys = protocol::HTTP::parse_query(request.path);

    auto file = load_file("data/html/login.html");
    file = std::vformat(file, std::make_format_args(querys["return_to"]));

    return {Version::HTTP_11,
            200,
            "OK",
            {{"content-type", "text/html"}, {"content-length", std::format("{}", file.size())}},
            file};
  });

  router.add(Method::POST, "/login", [&auth_client](Request& request) -> Response {
    auto body = protocol::HTTP::parse_query(request.body);
    auto auth_response = auth_client.transmit(protocol::ATP::Request{
        protocol::ATP::Type::LOG,
        protocol::ATP::LOG::Request{body["username"], body["password"], request.headers[HTTP_CLIENT_IP],
                                    request.headers["user-agent"]}});

    if (auth_response.type != protocol::ATP::Type::LOG)
    {
      Logger::error("auth response type did not match auth request type");
      return protocol::HTTP::standard_response(500);
    }

    auto session_id = std::get<protocol::ATP::LOG::Response>(auth_response.response).id;
    if (session_id)
    {
      Logger::debug("login successful");
      return {Version::HTTP_11,
              303,
              "See Other",
              {
                  {"set-cookie", HTTP_SESSION_ID "=" + (std::string)session_id.value()},
                  {"content-length", "0"},
                  {"location", "/" + body["return_to"]},
              },
              ""};
    }
    else
    {
      auto file = load_file("data/html/login.html");
      file = std::vformat(file, std::make_format_args(body["return_to"]));
      return {Version::HTTP_11,
              200,
              "OK",
              {{"content-type", "text/html"}, {"content-length", std::format("{}", file.size())}},
              file};
    }
  });

  ipc::TransportServer<protocol::HTTP> server(
      LOCAL_HOST, MIHON_SYNC_PORT, [&](Request _request) { return router.handle(_request); },
      protocol::HTTP::standard_response(500));
  server.listen();

  return 0;
}
