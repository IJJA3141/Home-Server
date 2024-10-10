#include "file/loading.hpp"
#include "log.hpp"
#include "server/reqres.hpp"
#include "server/router.hpp"
#include "server/server.hpp"
#include "test/test.hpp"
#include <string>

int main(int _argc, char *_argv[])
{
    test::request();

  /*Loader::init(_argv[0]);*/
  /**/
  /*// Loader::inject_file("static/js/test.js", {{"test", "str"}});*/
  /**/
  /*Router router;*/
  /*Tls https(&router, "/home/alexe/tmp/cert.pem", "/home/alexe/tmp/key.pem");*/
  /**/
  /*router.add(Method::GET, "/", [](Request _) -> Response {*/
  /*  return {{"HTTP/1.1", 200}, {}, Loader::load_file("index.html")};*/
  /*});*/
  /**/
  /*router.add(Method::GET, "/static/js/[file]", [](Request _) -> Response {*/
  /*  Response res;*/
  /**/
  /*  res.cmd = {"HTTP/1.1", 200};*/
  /*  try {*/
  /*    res.body = Loader::load_file("static/js/" + _.url_args["file"]);*/
  /*  } catch (std::exception _error) {*/
  /*    res.cmd.status_code = 404;*/
  /*  } catch (Loader::exception _error) {*/
  /*    res.cmd.status_code = 404;*/
  /*  };*/
  /**/
  /*  res.headers["Content-type"] = "text/javascript";*/
  /**/
  /*  return res;*/
  /*});*/
  /**/
  /*router.add(Method::GET, "/static/css/[file]", [](Request _) -> Response {*/
  /*  Response res;*/
  /**/
  /*  res.cmd = {"HTTP/1.1", 200};*/
  /*  try {*/
  /*    res.body = Loader::load_file("static/css/" + _.url_args["file"]);*/
  /*  } catch (std::exception _error) {*/
  /*    res.cmd.status_code = 404;*/
  /*  } catch (Loader::exception _error) {*/
  /*    res.cmd.status_code = 404;*/
  /*  };*/
  /**/
  /*  res.headers["Content-type"] = "text/css";*/
  /**/
  /*  return res;*/
  /*});*/
  /**/
  /*router.add(Method::GET, "/test", [](Request _) -> Response {*/
  /*  Response res;*/
  /**/
  /*  res.cmd = {"HTTP/1.1", 200};*/
  /**/
  /*  res.headers["Content-type"] = "application/octet-stream";*/
  /*  res.headers["Content-Disposition"] = "attachment; filename=\"filename.jpg\"";*/
  /**/
  /*  try {*/
  /*    res.body = Loader::load_file("static/test.jpg");*/
  /*  } catch (std::exception _error) {*/
  /*    res.cmd.status_code = 404;*/
  /*  } catch (Loader::exception _error) {*/
  /*    res.cmd.status_code = 404;*/
  /*  };*/
  /**/
  /*  return res;*/
  /*});*/
  /**/
  /*router.add(Method::GET, "/login", [](Request _req) -> Response {*/
  /*  Response res;*/
  /**/
  /*  std::map<std::string, std::string>::iterator user = _req.url_params.find("user");*/
  /*  std::map<std::string, std::string>::iterator password = _req.url_params.find("password");*/
  /**/
  /*  if (user == _req.url_params.end() && password == _req.url_params.end()) {*/
  /*    res.cmd = {"HTTP/1.1", 200};*/
  /*    res.headers["WWW-Authenticate"] = "Basic realm=\"User Visible Realm\", charset=\"UTF-8\"";*/
  /*    res.body = Loader::load_file("login.html");*/
  /*  } else if (user->second == "test" && password->second == "1234") {*/
  /*    res.cmd = {"HTTP/1.1", 200};*/
  /*    res.body = "200";*/
  /*  } else {*/
  /*    res.cmd = {"HTTP/1.1", 500};*/
  /*    res.headers["WWW-Authenticate"] = "Basic realm=\"User Visible Realm\", charset=\"UTF-8\"";*/
  /*    res.body = "err";*/
  /*  }*/
  /**/
  /*  return res;*/
  /*});*/
  /**/
  /*router.add_error_handler(Request::Failure::WRONGPATH, [](Request _) -> Response {*/
  /*  WARN("Path not found");*/
  /*  return {{"HTTP/1.1", 404}, {}, "there is nothing here"};*/
  /*});*/
  /**/
  /*router.add_error_handler(Request::Failure::UNAUTHORIZEDMETHOD, [](Request _) -> Response {*/
  /*  ERR("wrong method on ");*/
  /*  ERRV(_.cmd.path);*/
  /*  return {{"HTTP/1.1", 405}, {}, "there is nothing here"};*/
  /*});*/
  /**/
  /*https.bind(443);*/
  /*https.listen();*/
  /**/
  return 0;
}
