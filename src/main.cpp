#include "./params/params.hpp"
#include "./settings/settings.hpp"

#include <pstl/glue_execution_defs.h>
#include <string>
#include <unordered_map>

int main(int _argc, char *_argv[]) {
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



  return 0;
}
