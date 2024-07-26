#include <iostream>

#include "config.hpp"

Handler::Handler(std::string _key, std::function<void(std::string)> _lambda)
    : type(STRING), key(_key)
{
  this->fn = _lambda;
  return;
}
Handler::Handler(std::string _key, std::function<void(void)> _lambda) : type(VOID), key(_key)
{
  this->fn = [=](std::string) { _lambda(); };
  return;
}
Handler::Handler(std::string _key, std::function<void(int)> _lambda) : type(INT), key(_key)
{
  this->fn = [=](std::string _) { _lambda(atoi(_.c_str())); };
  return;
}

void parse(const std::vector<Arg> &_args, const std::vector<Handler> _handlers)
{
  for (const auto &arg : _args) {
    for (const auto &handler : _handlers) {
      if (arg.key == handler.key) {
#ifdef DEBUG
        std::cout << arg.key << " has been found." << std::endl;
#endif // DEBUG
        goto skip;
      }
    }

    std::cout << arg.key << " is not a command try --help" << std::endl;
    exit(1);
  skip:;
  }

  for (const auto &handler : _handlers)
    for (const auto &arg : _args)
      if (handler.key == arg.key) switch (handler.type) {
        case Handler::VOID:
          handler.fn("");
          break;
        case Handler::STRING:
        case Handler::INT:
          if (arg.arg != nullptr)
            handler.fn(*arg.arg);
          else
            std::cout << arg.key << " needs an argument" << std::endl;
          exit(2);
          break;
        }

  return;
}
