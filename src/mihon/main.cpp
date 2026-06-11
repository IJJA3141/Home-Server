#include "../auth/auth.hpp"
#include "../config.hpp"
#include "../ipc/transport/transport.hpp"
#include "../logger/logger.hpp"
#include "../routing/routing.hpp"
#include <netinet/tcp.h>

using Method = protocol::HTTP::Method;
using Version = protocol::HTTP::Version;
using Response = protocol::HTTP::Response;
using Request = protocol::HTTP::Request;
using Handler = protocol::Handler<protocol::HTTP>;

constexpr const char* slash(
    "<html lang=\"en\"> <head> <meta charset=\"utf-8\"> <title>Mihon Library Sync</title> <meta name=\"viewport\" "
    "content=\"width=device-width, initial-scale=1\"> </head> <body> <header> <h1>Mihon Library Sync</h1> "
    "<p>Private server for syncing Mihon libraries.</p> </header> <hr> <main> <section> <h2>Status</h2> <p>Server "
    "is online.</p> </section> <section> <h2>Endpoints</h2> <ul> <li><code>/upload</code> – Upload library "
    "data</li> <li><code>/download</code> – Download library data</li> <li><code>/status</code> – Server health "
    "check</li> </ul> </section> <section> <h2>Usage</h2> <ol> <li>Export your library from Mihon.</li> "
    "<li>Upload it to this server.</li> <li>Download it on another device to sync.</li> </ol> </section> </main> "
    "<hr> <footer> <p><small>Private service • No public access</small></p> </footer> </body> </html>");

constexpr const char* sync("<!DOCTYPE html>"
                           "<html lang=\"en\">"
                           "<head>"
                           "    <meta charset=\"UTF-8\">"
                           "    <title>Sync</title>"
                           "</head>"
                           "<body>"
                           "    <h1>Synchronization Status</h1>"
                           ""
                           "    <p>Current UUID:</p>"
                           ""
                           "    <pre>{}</pre>"
                           ""
                           "    <p>"
                           "        <a href=\"/\">Back to Home</a>"
                           "    </p>"
                           "</body>"
                           "</html>"

);

constexpr const char* login("<!DOCTYPE html>"
                            "<html lang=\"en\">"
                            "<head>"
                            "    <meta charset=\"UTF-8\">"
                            "    <title>Login</title>"
                            "</head>"
                            "<body>"
                            "    <h1>Login</h1>"
                            ""
                            "    <form method=\"post\" action=\"/login\">"
                            "        <div>"
                            "            <label for=\"username\">Username:</label><br>"
                            "            <input"
                            "                type=\"text\""
                            "                id=\"username\""
                            "                name=\"username\""
                            "                required>"
                            "        </div>"
                            ""
                            "        <br>"
                            ""
                            "        <div>"
                            "            <label for=\"password\">Password:</label><br>"
                            "            <input"
                            "                type=\"password\""
                            "                id=\"password\""
                            "                name=\"password\""
                            "                required>"
                            "        </div>"
                            ""
                            "        <br>"
                            ""
                            "        <button type=\"submit\">Login</button>"
                            "    </form>"
                            ""
                            "    <p>"
                            "        <a href=\"/\">Back to Home</a>"
                            "    </p>"
                            "</body>"
                            "</html>");

int main(void)
{
  Router router;
  ipc::TransportClient<protocol::ATP> auth_client(AUTH_IP, AUTH_PORT);
  auth_client.connect();

  protocol::Middleware<protocol::HTTP> auth_middleware =
      auth::create_auth_middleware("/login", "mihon", auth_client);

  router.add(Method::GET, "/", [](Request& _request) -> Response {
    auto res = Response{Version::HTTP_11,
                        200,
                        "OK",
                        {{"content-type", "text/html"}, {"content-length", std::format("{}", strlen(slash))}},
                        slash};

    return res;
  });

  router.add(Method::GET, "/static/*", [](Request&) -> Response { return {}; });

  router.add(Method::GET, "/sync", auth_middleware, [](Request& request) -> Response {
    auto uuid = "uuid := " + request.headers[HTTP_UUID];

    Logger::debug("uuid={}", uuid);

    auto str = std::format(sync, uuid);

    auto res = Response{Version::HTTP_11,
                        200,
                        "Ok",
                        {{"content-type", "text/plain"}, {"content-length", std::format("{}", str.size())}},
                        str};

    return res;
  });

  router.add(Method::GET, "/login", [](Request& _request) -> Response {
    auto res = protocol::HTTP::Response{
        Version::HTTP_11,
        200,
        "OK",
        {{"content-type", "text/html"}, {"content-length", std::format("{}", strlen(login))}},
        login};

    return res;
  });

  router.add(Method::POST, "/login", [&auth_client](Request& _request) -> Response {
    //  username=test&password=1234
    // a bit cheesy but works
    size_t start = _request.body.find('=');
    size_t size = _request.body.find('&') - start;
    auto user = _request.body.substr(start, size);

    start = _request.body.find_last_of('=');
    auto pwd = _request.body.substr(start);
    auto req = protocol::ATP::LOG::Request{.user = user,
                                           .pwd = pwd,
                                           .ip = _request.headers[HTTP_CLIENT_IP],
                                           .user_agent = _request.headers["user-agent"]};

    auto res = auth_client.transmit(protocol::ATP::Request{
        .type = protocol::ATP::Type::LOG,
        .request = req,
    });

    if (res.type != protocol::ATP::Type::LOG) throw;

    auto re = std::get<protocol::ATP::LOG::Response>(res.response);
    protocol::HTTP::Response r;
    if (re.id)
    {
      r = protocol::HTTP::Response{
          Version::HTTP_11,
          200,
          "OK",
          {{"content-type", "text/plain"}, {"content-length", std::format("{}", Uuid::UNPARSED_SIZE)}},
          re.id.value()};
    }
    else
    {
      r = protocol::HTTP::Response{
          Version::HTTP_11,
          200,
          "OK",
          {{"content-type", "text/plain"}, {"content-length", std::format("{}", re.id.error().size())}},
          re.id.error()};
    }

    return r;
  });

  ipc::TransportServer<protocol::HTTP> server(
      LOCAL_HOST, MIHON_SYNC_PORT, [&](Request _request) { return router.handle(_request); },
      protocol::HTTP::standard_response(500));
  server.listen();

  return 0;
}
