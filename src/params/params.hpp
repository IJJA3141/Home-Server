#ifndef PARAMS
#define PARAMS

#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <vector>

class Parser {
public:
  Parser(int _argc, char _argv[]);
  bool find(std::string _str);

private:
  std::map<std::string, std::vector<std::string>> map_;
};

#endif // !PARAMS
