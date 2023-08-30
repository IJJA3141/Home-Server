#include "./params.hpp"

Parser::Parser() {
  this->map_ = {};

  return;
}

void Parser::add(const char *_pChar, std::function<void(char *_char)> _fn) {
  this->map_[*_pChar] = _fn;

  return;
}

void Parser::parse(int _argc, char *_argv[]) {
  char *str;

  for (int i = 1; i < _argc; i++) {
    if (this->map_.find(*_argv[i]) != this->map_.end()) {
      this->map_[*_argv[i]](str);
    } else
      str = _argv[i];
  }

  return;
}
