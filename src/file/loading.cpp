#include "loading.hpp"
#include "../log.hpp"
#include <cassert>
#include <fstream>
#include <string>

bool Loader::initialized_ = false;
std::string Loader::path_ = "";

void Loader::init(std::string _arg)
{
  Loader::path_ = _arg.substr(0, _arg.rfind('/') + 1);
  Loader::initialized_ = true;

  return;
}

std::string Loader::load_file(const std::string &_path)
{
  if (!Loader::initialized_) {
    ERR("file loader was used before getting initialized...");
    throw Loader::exception::UNINITIALIZED;
  }

  LOG("loading " + Loader::path_ + _path + "...");
  std::ifstream stream((Loader::path_ + _path));

  if (stream.is_open()) {
    std::string str((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    stream.close();

    LOG("file loaded succesfully");
    return str;
  }

  VERBERR("file opening failed");
  throw Loader::exception::FILE;
}

std::string Loader::inject_file(const std::string &_path,
                                const std::map<std::string, std::string> &_replacement)
{
  if (!Loader::initialized_) {
    ERR("file loader was used before getting initialized...");
    throw Loader::exception::UNINITIALIZED;
  }

  LOG("loading and injecting " + Loader::path_ + _path + "...");
  std::ifstream stream((Loader::path_ + _path));
  std::string content;

  if (stream.is_open()) {
    std::string str;

    while (std::getline(stream, str) && str.substr(0, 6) == "const ") {
      auto it = _replacement.find(str.substr(6, str.find(" ", 7) - 6));

      if (it == _replacement.end()) {
        ERR("no matching key found");
      } else {
        content += "const " + it->first + " = \"" + it->second + "\"\n";
      };

      content +=
          std::string((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

      PRINT(content);
    }

    stream.close();

    LOG("file loaded succesfully");
    return str;
  }

  VERBERR("file opening failed");
  throw Loader::exception::FILE;
}
