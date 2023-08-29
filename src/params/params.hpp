#ifndef PARAMS
#define PARAMS

#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Parser {
public:
  std::unordered_map<char, std::vector<char>> map;
  
  Parser(const char *_err, std::vector<const char *> _vStr);
  void parse(int _argc, char* _argv[]);
  bool find(const char *_pChar);

  std::vector<const char *> vStr_;
private:
  const char *err_;
};

#endif // !PARAMS
