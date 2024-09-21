#pragma once

#include <string>

class Loader
{
public:
  enum Exeptoin { UNINITIALIZED, FILE };

  Loader() = delete;

  static void init(std::string _arg);
  static std::string load_file(const std::string &_path);

private:
  static std::string path_;
  static bool initialized_;
};
