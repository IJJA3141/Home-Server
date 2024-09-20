#include "log.hpp"
#include "server/reqres.hpp"
#include "server/router.hpp"
#include "server/server.hpp"
#include <fstream>
#include <string>

static std::string path;

std::string load_file(const std::string _path)
{
  LOG("loading file...");
  std::ifstream steam((path + _path));
  if (!steam.is_open()) VERBERR("file opening failed");

  std::string str((std::istreambuf_iterator<char>(steam)), std::istreambuf_iterator<char>());
  steam.close();

  LOG("file loaded succesfully");
  return str;
}

int main(int _argc, char *_argv[])
{
  Router router;

  Tcp http(&router);

  router.add(Method::GET, "/static/js/[file]", [](Request _) -> Response {
    PRINT("FROM /static/js/[file]")
    return {{"HTTP/1.1", 200}, {}, "test " + _.url_args["file"] + "\n"};
  });

  router.add(Method::GET, "/static/css/[file]", [](Request _) -> Response {
    PRINT("FROM /static/css/[file]")
    return {{"HTTP/1.1", 200}, {}, "test " + _.url_args["file"] + "\n"};
  });

  router.add(Method::GET, "/", [](Request _) -> Response {
    PRINT("FROM /")
    return {{"HTTP/1.1", 200}, {}, load_file("index.html")};
  });

  http.bind(80);
  http.listen();

  return 0;
}
