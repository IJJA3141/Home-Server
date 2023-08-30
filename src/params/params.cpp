#include "./params.hpp"
#include <algorithm>
#include <cstdlib>
#include <cstring>

Parser::Parser(const char *_err, std::vector<const char *> _vStr) {
  this->err_ = _err;
  this->vStr_ = _vStr;
  return;
}

bool Parser::find(const char *_pChar) {
  auto l = [=](const char *__pChar) {
    return strcmp(__pChar, _pChar) == 0;
  };

  return (std::find_if(this->vStr_.begin(), this->vStr_.end(), l) !=
          this->vStr_.end());
}

void Parser::parse(int _argc, char *_argv[]) {
  int last = -1;

  for (int i = 1; i < _argc; i++) {
    if (this->find(_argv[i])) {
      std::cout << "New key: " << _argv[i] << "\n";
      last = i;
      this->map.insert({*_argv[i], {}});
    } else {
      if (last == -1) {
        std::cout << this->err_ << std::endl;
        abort();
      }

      std::cout << "New value: " << _argv[i] << "\n";
      this->map[*_argv[last]].push_back(*_argv[i]);
    }
  }

  return;
}
