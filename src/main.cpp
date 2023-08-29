#include "./params/params.hpp"
#include "./settings/settings.hpp"

#include <pstl/glue_execution_defs.h>
#include <string>
#include <unordered_map>

int main(int _argc, char *_argv[]) {

  std::cout << _argc << "\n" << *_argv << "\n\n__________" << std::endl;

  const char *err = "err";

  Parser parser = Parser(err, {
                                  "--help",
                                  "--http-port",
                                  "--https-port",
                                  "--cert-path",
                                  "--key-path",
                                  "--no-save",
                                  "-h",
                              });

  parser.parse(_argc, _argv);

  std::cout << "\n\n\n\n\n__________________" << std::endl;

  if (parser.find("--help") || parser.find("-h"))
    std::cout << "help" << std::endl;
  if (parser.find("--http-port"))
    std::cout << parser.map[*"--http-port"].size() << std::endl;
  if (parser.find("--https-port"))
    std::cout << parser.map[*"--https-port"].size() << std::endl;
  if (parser.find("--cert-path"))
    std::cout << parser.map[*"--cert-path"].size() << std::endl;
  if (parser.find("--key-path"))
    std::cout << parser.map[*"--key-path"].size() << std::endl;
  if (parser.find("--no-save"))
    std::cout << parser.map[*"--no-save"].size() << std::endl;

  std::cout << "\n\n\n\n\n__________________" << std::endl;

  for(int i = 0; i < parser.vStr_.size(); i++){
    std::cout << parser.vStr_[i] << std::endl;
  }

  return 0;
}
