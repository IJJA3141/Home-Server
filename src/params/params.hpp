#ifndef PARAMS
#define PARAMS

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

class Params {
public:
  std::string http;
  std::string https;
  std::string key;
  std::string cert;

  Params(int _argc, char *_argv[]);

private:
  bool reset_;

  void Load_();
  void Save_();
};

#endif // !PARAMS
