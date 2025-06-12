#include "log.hpp"

#include <iostream>
#include <map>
#include <string>
#include <vector>

int main(int argc, char* argv[])
{
  std::cout << "Hello world !" << std::endl;

  log("log");

  std::vector<std::string> vec = {"vec0", "vec1"};
  warn(vec);

  std::map<std::string, std::string> map = {{"key0", "map0"}, {"key1", "map1"}, {"key2", "map2"}};
  err(map);

  assert(false, "this was bad...");

  return 0;
}
