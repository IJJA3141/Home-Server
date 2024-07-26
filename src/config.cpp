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

void parse(int _argc, char *_argv[], const std::vector<Handler> _handlers)
{
  // i start at 1 to skip path
  for (size_t i = 1; i < _argc; i++) {
    for (auto &handler : _handlers) {
      if (_argv[i] == handler.key){ 
        continue;
      };
    }

    exit(1);
  };

  for (const Handler &handler : _handlers) {
    for (size_t i = 0; i < _argc; i++) {
      if (_argv[i] == handler.key) {
        switch (handler.type) {
        case Handler::VOID:
          handler.fn("");
          break;

        case Handler::STRING:
        case Handler::INT:
          if (++i < _argc)
            handler.fn(_argv[i]);
          else
            exit(2);
          break;
        }
      }
    };
  }

  return;
}


