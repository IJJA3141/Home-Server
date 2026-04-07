#include "implementation.hpp"
#include "log.hpp"
#include "network/auth.hpp"
#include "network/http.hpp"
#include "network/router.hpp"
#include "network/server.hpp"

#include <filesystem>
#include <map>
#include <string>
#include <thread>
#include <uuid/uuid.h>

// TODO parse cookies

#define ROUTERS_ADD(pack...)                                                                                      \
  http_router.add(pack);                                                                                          \
  https_router.add(pack)

int main(int argc, char* argv[])
{
  std::filesystem::path cwd = std::filesystem::current_path();
  Authenticator auth(10, cwd / "data", std::chrono::hours{3 * 24});

  Router http_router(handler::internal_error);
  Router https_router(handler::internal_error);

  ROUTERS_ADD(http::Error::I_URL, handler::not_found);
  ROUTERS_ADD(http::Error::I_METHOD, handler::method_not_allowed);

  ROUTERS_ADD(http::Method::GET, "/static/", [=](http::Request _request, std::string _path) -> http::Response {
    return Response(HTTP_OK, load(cwd / "public" / "static" / _path), _request.cmd.protocol);
  });

  ROUTERS_ADD(http::Method::GET, "/", Response(HTTP_OK, load(cwd / "public" / "main" / "main.html")));
  https_router.add(http::Method::GET, "/login/", Response(HTTP_OK, load(cwd / "public" / "login" / "login.html")));

  https_router.add(http::Method::POST, "/login/", [&](http::Request _request) -> http::Response {
    std::map<std::string, std::string> query;

    if (http::parse_querys(_request.body, query))
    {
      err("failed to parse login");
      return handler::unauthorized;
    }

    if (query["user"] == "" || query["password"] == "")
    {
      // err("empty user or password field", query);
      return handler::unauthorized;
    }

    if (auth.invalidate_password(query["user"], query["password"]))
    {
      log("invalid password for user:", query["user"], "password:", query["password"]);
      return handler::login_failed;
    }

    log(query["user"], "succesfuly loged in.");
    Session session = auth.generate(query["user"]);
    std::string str;
    str << session;
    std::string red = _request.cmd.url.querys["redirect"];
    if (red == "") red = "/";

    return {
        .protocol = _request.cmd.protocol,
        .status = HTTP_FOUND,
        .headers =
            {
                {"Strict-Transport-Security", "max-age=31536000"}, // 1 year
                {"Set-Cookie", str},
                {"Location", red},
            },
    };
  });

  https_router.add(http::Method::GET, "/api/user/banner/", [&](http::Request _request) -> http::Response {
    uuid_t uuid;
    if (uuid_parse(_request.headers["session"].c_str(), uuid) == 0)
    {
      int index = auth.fetch(uuid);
      if (index != -1)
      {
        Session session = auth[index];

        return Response(HTTP_OK, "<p>" + session.user + "</p>", _request.cmd.protocol);
      }
      {
        debug("failed to log");
      }
    }
    else
    {
      debug("failed to parse", _request.headers["session"].c_str(), "<--");
    }

    return handler::unauthorized;
  });

  Tcp http_server(80, http_router);
  Tls https_server(443, "/home/alexe/tmp/cert.pem", "/home/alexe/tmp/key.pem", https_router);

  std::thread http_thread([&]() { http_server.listen(); });
  std::thread https_thread([&]() { https_server.listen(); });

  http_thread.join();
  https_thread.join();

  return 0;
}
