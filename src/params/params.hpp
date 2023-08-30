#ifndef PARAMS
#define PARAMS

#include <algorithm>
#include <cstring>
#include <functional>
#include <map>

class Parser {
public:
  Parser();
  void add(const char *_pChar, std::function<void(char *_char)> _fn);
  void parse(int _argc, char *_argv[]);

private:
  std::map<const char, std::function<void(char *_char)>> map_;
};

#endif // !PARAMS
