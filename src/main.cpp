#include "log.hpp"
#include "server/router.hpp"
#include "server/server.hpp"
#include "test/test.hpp"
#include <fstream>

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
  test::router();

  return 0;
}
