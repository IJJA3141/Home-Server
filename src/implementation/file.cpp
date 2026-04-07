#include "../implementation.hpp"
#include "../log.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>

std::string load(std::filesystem::path _path)
{
  std::ifstream stream(_path);
  assert(stream.is_open(), "could not find", _path);

  std::stringstream buff;
  buff << stream.rdbuf();

  return buff.str();
}
