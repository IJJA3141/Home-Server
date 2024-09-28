#include "file/loading.hpp"
#include "log.hpp"
#include "server/reqres.hpp"
#include "server/router.hpp"
#include "server/server.hpp"
#include <string>

int main(int _argc, char *_argv[])
{
  Loader::init(_argv[0]);

  Loader::inject_file("static/js/test.js", {{"test", "str"}});

  /*
  Router router;
  Tcp http(&router);

  router.add(Method::GET, "/", [](Request _) -> Response {
    return {{"HTTP/1.1", 200}, {}, Loader::load_file("index.html")};
  });

  router.add(Method::GET, "/static/js/[file]", [](Request _) -> Response {
    Response res;

    res.cmd = {"HTTP/1.1", 200};
    try {
      res.body = Loader::load_file("static/js/" + _.url_args["file"]);
    } catch (std::exception _error) {
      res.cmd.status_code = 404;
    } catch (Loader::exception _error) {
      res.cmd.status_code = 404;
    };

    res.headers["Content-type"] = "text/javascript";

    return res;
  });

  router.add(Method::GET, "/static/css/[file]", [](Request _) -> Response {
    Response res;

    res.cmd = {"HTTP/1.1", 200};
    try {
      res.body = Loader::load_file("static/css/" + _.url_args["file"]);
    } catch (std::exception _error) {
      res.cmd.status_code = 404;
    } catch (Loader::exception _error) {
      res.cmd.status_code = 404;
    };

    res.headers["Content-type"] = "text/css";

    return res;
  });

  router.add(Method::GET, "/test", [](Request _) -> Response {
    Response res;

    res.cmd = {"HTTP/1.1", 200};

    res.headers["Content-type"] = "application/octet-stream";
    res.headers["Content-Disposition"] = "attachment; filename=\"filename.jpg\"";

    try {
      res.body = Loader::load_file("static/test.jpg");
    } catch (std::exception _error) {
      res.cmd.status_code = 404;
    } catch (Loader::exception _error) {
      res.cmd.status_code = 404;
    };

    return res;
  });

  router.add(Method::GET, "/login", [](Request _) -> Response {
    if (_.connection_type == Client::Type::STANDARD) {
      // return {{"HTTP/1.1", 401}, {}, "Unauthorized"};
    }

    Response res;

    res.cmd = {"HTTP/1.1", 200};
    res.headers["RSA"] = "test value";
    res.headers["Content-type"] = "text/html";
    res.body = Loader::load_file("login.html");

    return res;
  });

  router.add_error_handler(Request::Failure::WRONGPATH, [](Request _) -> Response {
    LOG("Path not found");
    return {{"HTTP/1.1", 404}, {}, "there is nothing here"};
  });

  http.bind(80);
  http.listen();
  */

  return 0;
}
