#include "../config.hpp"
#include "../ipc/transport/transport.hpp"
#include "../routing/routing.hpp"

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

int main(void)
{
  Router router;

  router.add(Method::GET, "/", [](Request& _request) -> Response {
    return {Version::HTTP_11,
            200,
            "OK",
            {{"content-type", "text/html"}, {"content-length", std::format("{}", strlen(slash))}},
            slash};
  });

  router.add(Method::GET, "/static/*", [](Request&) -> Response { return {}; });

  ipc::TransportServer<protocol::HTTP> server(
      LOCAL_HOST, MIHON_SYNC_PORT, [&](Request _request) { return router.handle(_request); },
      protocol::HTTP::standard_response(500));
  server.listen();

  return 0;
}
