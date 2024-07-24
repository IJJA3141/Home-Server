#include <iostream>

#include "config.hpp"

Parser::Parser(int _argc, char *_argv[], std::vector<std::string> _keys)
{
  std::string str;

  // Start at 1 to skip executable's path
  for (size_t i = 1; i < _argc; i++) {
    str = _argv[i];

    if (str[0] == '-') {
      bool in = false;
      for (auto key : _keys)
        if (key == str) in = true;

      if (!in) {
        std::cout << "not in" << std::endl;
        exit(1);
      }
    }

    this->args_.push_back(str);
  }

  return;
}

void Parser::parse(std::pair<std::string, std::string> _matches, std::function<void(void)> _lambda)
{
  for (auto arg : this->args_) {
    if (arg == _matches.first || arg == _matches.second) {
      _lambda();
      break;
    }
  }

  return;
}

void Parser::parse(std::pair<std::string, std::string> _matches, std::function<void(int)> _lambda)
{
  for (size_t i = 0; i < this->args_.size(); i++) {
    if (this->args_[i] == _matches.first || this->args_[i] == _matches.second) {
      if (++i < this->args_.size()) {
        _lambda(stoi(this->args_[i]));
        break;
      } else {
        std::cout << "err arg?" << std::endl;
        exit(1);
      }
    }
  }

  return;
}

void Parser::parse(std::pair<std::string, std::string> _matches,
                   std::function<void(std::string)> _lambda)
{
  for (size_t i = 0; i < this->args_.size(); i++) {
    if (this->args_[i] == _matches.first || this->args_[i] == _matches.second) {
      if (++i < this->args_.size()) {
        _lambda(this->args_[i]);
        break;
      } else {
        std::cout << "err args ?" << std::endl;
        exit(1);
      }
    }
  }

  return;
}
