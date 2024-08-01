#include "option.hpp"

#ifdef DEBUG
#include <iostream>
#endif

Option::Option(int _shortName, std::string _longName, std::function<void(std::string)> _func)
    : func(_func), longName(_longName), shortName(_shortName), hasArg(true){};

Option::Option(int _shortName, std::string _longName, std::function<void(void)> _func)
    : longName(_longName), shortName(_shortName), hasArg(false)
{
  this->func = [_func](std::string) { _func(); };
};

Option::Option(int _shortName, std::string _longName, std::function<void(int)> _func)
    : longName(_longName), shortName(_shortName), hasArg(true)
{
  this->func = [_func](std::string _) { _func(atoi(_.c_str())); };
}

Parser::Parser(int _argc, char *_argv[], std::vector<std::string> _scmds, std::vector<Option> _opts)
{
  // i start at one to skip path
  size_t i = 1;

  for (; i < _argc; i++) {
    for (const auto &scmd : _scmds) {
      if (_argv[i] == scmd) {
        this->scmd = _argv[i++];
        goto end;
      }
    }

    this->pre_.push_back(_argv[i]);
  }

end:
  for (; i < _argc; i++)
    this->post_.push_back(_argv[i]);

#ifdef DEBUG
  std::cout << "\n[DEBUG]pre scmd:\n";
  for (const auto &a : this->pre_) {
    std::cout << "[DEBUG]" << a << "\n";
  }
  std::cout << "\n[DEBUG]scmd:\n";
  std::cout << "[DEBUG]" << this->scmd << "\n\n[DEBUG]post scmd:\n";
  for (const auto &a : this->post_) {
    std::cout << "[DEBUG]" << a << "\n";
  }
  std::cout << std::endl;
#endif // DEBUG

  std::reverse(this->pre_.begin(), this->pre_.end());

  std::string arg;
  while (!this->pre_.empty()) {
    arg = this->pre_.back();

    if (arg.size() > 1 && arg[0] == '-') {
      if (arg.size() > 2 && arg[1] == '-') {
        // is long arg

        size_t pos = arg.find('=');
        if (pos != std::string::npos) {
          // a value is assigned

          for (const auto &opt : _opts) {
            if (arg.compare(2, pos, opt.longName) == 0) {
              opt.func(arg.substr(pos + 1));
              goto skip;
            }
          }

          exit(4); // not a long name
        } else {
          // no assigned value

          for (const auto &opt : _opts) {
            if (arg.compare(2, arg.size() - 2, opt.longName) == 0) {
              if (opt.hasArg) {
                this->pre_.pop_back();
                if (this->pre_.empty()) exit(6); // need arg

                opt.func(this->pre_.back());
              } else {
                opt.func("");
              }

              goto skip;
            }
          }

          exit(5); // not a long name
        }
      } else {
        // is short arg

        // start at one to skip -
        for (size_t i = 1; i < arg.size(); i++) {
          for (const auto &opt : _opts) {
            if (arg[i] == opt.shortName) {
              if (opt.hasArg) {
                if (++i >= arg.size()) {
                  this->pre_.pop_back();
                  if (this->pre_.empty()) exit(2); // need arg

                  opt.func(this->pre_.back());
                } else {
                  opt.func(arg.substr(i));
                }
                goto skip;
              } else {
                opt.func("");
                goto next;
              }
            }
          }

          exit(1); // not a short name
        next:;
        }
      }
    } else {
      std::cout << arg << " is not an option." << std::endl;
      exit(3); // the fuck is this
    };

  skip:
    this->pre_.pop_back();
  }

  return;
};

void Parser::parse(std::vector<Option> _opts) {}
