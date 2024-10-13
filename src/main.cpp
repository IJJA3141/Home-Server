#include "file/loading.hpp"
#include "log.hpp"
#include "server/reqres.hpp"
#include "server/router.hpp"
#include "server/server.hpp"

int main(int _argc, char *_argv[])
{
  Router router;
  Tls https(&router, "/home/alexe/tmp/cert.pem", "/home/alexe/tmp/key.pem");
  Tcp http(&router);

  Loader::init(_argv[0]);

  // static file access
  router.add(Method::GET, "/static/js/[file]", [](Request _req) -> Response {
    Response res;

    try {
      res.cmd = {"HTTP/1.1", 200};
      res.body = Loader::load_file("static/js/" + _req.url_args["file"]);
    } catch (std::exception _error) {
      res.cmd.status_code = 404;
      return res;
    } catch (Loader::exception _error) {
      res.cmd.status_code = 404;
      return res;
    };

    res.headers["Content-type"] = "text/javascript";

    return res;
  });

  router.add(Method::GET, "/static/css/[file]", [](Request _req) -> Response {
    Response res;

    try {
      res.cmd = {"HTTP/1.1", 200};
      res.body = Loader::load_file("static/css/" + _req.url_args["file"]);
    } catch (std::exception _error) {
      res.cmd.status_code = 404;
      return res;
    } catch (Loader::exception _error) {
      res.cmd.status_code = 404;
      return res;
    };

    res.headers["Content-type"] = "text/css";

    return res;
  });

  // paths
  router.add(Method::GET, "/", [](Request _req) -> Response {
    PRINTM(_req.headers);

    return {{"HTTP/1.1", 200}, {}, Loader::load_file("index.html")};
  });

  router.add(Method::GET, "/test", [](Request _req) -> Response {
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

  router.add(Method::GET, "/login", [](Request _req) -> Response {
    Response res;

    std::map<std::string, std::string>::iterator auth = _req.headers.find("Authorization");

    if (auth == _req.headers.end()) {
      LOG("login request")
      res.cmd = {"HTTP/1.1", 200};
      res.headers["WWW-Authenticate"] = "Basic realm=\"User Visible Realm\", charset=\"UTF-8\"";
      res.body = Loader::load_file("login.html");
      return res;

    } else if (auth->second == "Basic dGVzdDoxMjM0") {
      LOG("auth request passed\nuser: " + auth->second)
      res.cmd = {"HTTP/1.1", 200};
      res.headers["Host"] = "localhost";
      res.headers["Set-Cookie"] = "uuid=123456789; Max-Age=60; SameSite=Strict; Secure; HttpOnly";

    } else {
      LOG("auth failed")
      res.cmd = {"HTTP/1.1", 407};
      res.headers["WWW-Authenticate"] = "Basic realm=\"User Visible Realm\", charset=\"UTF-8\"";
      res.headers["Set-Cookie"] = "uuid=; Max-Age=-1; SameSite=Strict; Secure; HttpOnly";
      res.body = "no user";
    }

    return res;
  });

  router.add(Method::GET, "/user", [](Request _req) -> Response {
    Response res;

    PRINTM(_req.headers)

    std::map<std::string, std::string>::iterator auth = _req.headers.find("Cookie");

    res.cmd = {"HTTP/1.1", 200};
    res.body = "{\"logged\":false,\"username\":\"\"}";

    if (auth != _req.headers.end() && auth->second == "uuid=123456789") {
      res.body = "{\"logged\":true,\"username\":\"IJJA3141\"}";
    }

    res.headers["Content-type"] = "application/json";

    return res;
  });

  // error handling
  router.add_error_handler(Request::Failure::WRONGPATH, [](Request _req) -> Response {
    WARN("Path not found");
    return {{"HTTP/1.1", 404}, {}, "there is nothing here"};
  });

  router.add_error_handler(Request::Failure::UNAUTHORIZEDMETHOD, [](Request _req) -> Response {
    ERR("wrong method on ");
    ERRV(_req.cmd.path);
    return {{"HTTP/1.1", 405}, {}, "there is nothing here"};
  });

  // server start
  https.bind(443);
  https.listen();

  return 0;
}
