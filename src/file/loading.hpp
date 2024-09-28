#pragma once

#include <map>
#include <string>

class Loader
{
public:
  enum exception { UNINITIALIZED, FILE };

  Loader() = delete;

  static void init(std::string _arg);
  static std::string load_file(const std::string &_path);
  static std::string inject_file(const std::string &_path,
                                 const std::map<std::string, std::string> &_replacement);

private:
  static std::string path_;
  static bool initialized_;
};
