#include "file/loading.hpp"
#include "log.hpp"
#include "server/reqres.hpp"
#include "server/router.hpp"
#include "server/server.hpp"
#include <fstream>
#include <string>

int main(int _argc, char *_argv[])
{
  /*Router router;*/
  /**/
  /*Tcp http(&router);*/
  /**/
  /*router.add(Method::GET, "/static/js/[file]", [](Request _) -> Response {*/
  /*  PRINT("FROM /static/js/[file]")*/
  /*  return {{"HTTP/1.1", 200}, {}, "test " + _.url_args["file"] + "\n"};*/
  /*});*/
  /**/
  /*router.add(Method::GET, "/static/css/[file]", [](Request _) -> Response {*/
  /*  PRINT("FROM /static/css/[file]")*/
  /*  return {{"HTTP/1.1", 200}, {}, "test " + _.url_args["file"] + "\n"};*/
  /*});*/
  /**/
  /*router.add(Method::GET, "/", [](Request _) -> Response {*/
  /*  PRINT("FROM /")*/
  /*  return {{"HTTP/1.1", 200}, {}, load_file("index.html")};*/
  /*});*/
  /**/
  /*http.bind(80);*/
  /*http.listen();*/

  try {
    Loader::load_file("");
  } catch (std::exception e) {
  } catch (Loader::Exeptoin e) {
  }

  return 0;
}
