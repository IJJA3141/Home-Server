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
  std::string path_;

  void load_();
  void save_();
};

#endif // !PARAMS
