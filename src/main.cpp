#include "log.hpp"
#include "server/router.hpp"
#include "server/server.hpp"
#include <fstream>

static std::string path;

std::string load_file(const std::string _path)
{
  std::ifstream steam((path + _path));
  if (!steam.is_open()) VERBERR("file opening failed");

  std::string str((std::istreambuf_iterator<char>(steam)), std::istreambuf_iterator<char>());
  steam.close();

  PRINT(str);
  return str;
}

int main(int _argc, char *_argv[])
{
  Router router;
  path = _argv[0];
  path = path.substr(0, path.rfind("/"));

  Tcp http(&router);

  router.add(Method::GET, "/", [](Request _req) -> Response {
    Response res;
    res.cmd.status_code = 200;
    res.cmd.protocol = _req.cmd.protocol;
    res.body = load_file("/html/index.html");

    return res;
  });

  router.add(Method::GET, "/test/[user]", [](Request _req) -> Response {
    Response res;
    res.cmd.status_code = 200;
    res.cmd.protocol = _req.cmd.protocol;
    res.body = load_file("/html/user.html");

    LOG(_req.url_args["user"]);

    return res;
  });

  router.add(Method::GET, "/css/style.css", [](Request _req) -> Response {
    Response res;
    res.cmd.status_code = 200;
    res.cmd.protocol = _req.cmd.protocol;
    res.body = load_file("/css/style.css");

    return res;
  });

  http.bind(80);
  http.listen();

  return 0;
}
