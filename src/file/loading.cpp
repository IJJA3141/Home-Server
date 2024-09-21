#include "loading.hpp"
#include "../log.hpp"
#include <cassert>
#include <fstream>

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
    throw Loader::Exeptoin::UNINITIALIZED;
  }

  std::ifstream steam((Loader::path_ + _path));
  LOG("loading " + Loader::path_ + _path + "...");

  if (steam.is_open()) {
    std::string str((std::istreambuf_iterator<char>(steam)), std::istreambuf_iterator<char>());
    steam.close();

    LOG("file loaded succesfully");
    return str;
  }

  VERBERR("file opening failed");
  throw Loader::Exeptoin::FILE;
}
