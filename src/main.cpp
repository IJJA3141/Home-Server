#include "log.hpp"
#include "network/http.hpp"
#include "network/router.hpp"
#include "network/server.hpp"
#include "res.hpp"

#include <openssl/err.h>
#include <string>
#include <thread>

http::Response func(http::Request _req)
{
  debug((std::string)_req);

  http::Response res;

  res.protocol = http::Protocol::HTTP_11;
  res.status = HTTP_OK;
  res.type = HTTP_MIME_HTML;

  res.body =
      "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\" /><meta name=\"viewport\" "
      "content=\"width=device-width, initial-scale=1.0\"/><title>Welcome Page</title><style>body { margin: 0; padding: "
      "0; font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background-color: #f0f4f8; display: flex; "
      "align-items: center; justify-content: center; height: 100vh; } .welcome-container { text-align: center; "
      "background: white; padding: 40px; border-radius: 10px; box-shadow: 0 10px 25px rgba(0, 0, 0, 0.1); } h1 { "
      "color: "
      "#333; } p { color: #666; margin-top: 10px; } </style> </head> <body> <div class=\"welcome-container\"> "
      "<h1>Welcome to Our Website!</h1> <p>We're glad you're here. Explore and enjoy your stay.</p> </div> </body> "
      "</html>";

  return res;
}

http::Response mv(http::Request _req)
{
  http::Response res{
      .protocol = http::Protocol::HTTP_11,
      .status = HTTP_MOVED_PERMANENTLY,
      .type = HTTP_MIME_PLAIN,
  };

  res.body = "moved to somewere else";

  return res;
}

int main(int argc, char* argv[])
{
  Router http_router(READ_ERROR);
  Router https_router(READ_ERROR);

  http_router.add(http::Method::GET, "/", &mv);
  https_router.add(http::Method::GET, "/", func);

  Tcp http_server(80, http_router);
  std::thread http_thread([&]() -> void { http_server.listen(); });

  Tls https_server(443, "./cert.pem", "./key.pem", https_router);
  std::thread https_thread([&]() -> void { https_server.listen(); });

  http_thread.join();
  https_thread.join();

  return 0;
}
