#include <iostream>

#include "config.hpp"

Parser::Parser(int _argc, char *_argv[])
{
  for (size_t i = 0; i < _argc; i++)
    this->_args.push_back(_argv[i]);

  return;
}

void Parser::parse(std::pair<std::string, std::string> _matches, std::function<void(void)> _lambda)
{
  for (auto arg : this->_args)
    if (arg == _matches.first || arg == _matches.second) _lambda();

  return;
}

void Parser::parse(std::pair<std::string, std::string> _matches, std::function<void(int)> _lambda)
{
  for (size_t i = 0; i < this->_args.size(); i++) {
    if (this->_args[i] == _matches.first || this->_args[i] == _matches.second) {
      if (++i < this->_args.size()) {
        _lambda(stoi(this->_args[i]));
        break;
      } else {
        std::cout << "err" << std::endl;
        exit(1);
      }
    }
  }

  return;
}

void Parser::parse(std::pair<std::string, std::string> _matches, std::function<void(bool)> _lambda)
{
  for (size_t i = 0; i < this->_args.size(); i++) {
    if (this->_args[i] == _matches.first || this->_args[i] == _matches.second) {
      if (++i < this->_args.size()) {
        if (this->_args[i] == "true") {
          _lambda(true);
        } else if (this->_args[i] == "false") {
          _lambda(false);
        } else {
          std::cout << "err" << std::endl;
          exit(1);
        }
        break;
      } else {
        std::cout << "err" << std::endl;
        exit(1);
      }
    }
  }

  return;
}

void Parser::parse(std::pair<std::string, std::string> _matches,
                   std::function<void(std::string)> _lambda)
{
  for (size_t i = 0; i < this->_args.size(); i++) {
    if (this->_args[i] == _matches.first || this->_args[i] == _matches.second) {
      if (++i < this->_args.size()) {
        _lambda(this->_args[i]);
        break;
      } else {
        std::cout << "err" << std::endl;
        exit(1);
      }
    }
  }

  return;
}
