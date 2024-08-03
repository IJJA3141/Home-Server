#include "option.hpp"
#include "../log.hpp"
#include <iterator>

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
    : optv_(_opts)
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

  PRINT("pre scmd");
  PRINTV(this->pre_);
  PRINT("scmd");
  PRINT(this->scmd);
  PRINT("post scmd");
  PRINTV(this->post_);

  std::reverse(this->pre_.begin(), this->pre_.end());
  std::reverse(this->post_.begin(), this->post_.end());

  this->parse(this->pre_, _opts);

  return;
};

void Parser::parse(std::vector<std::string> &_argv, const std::vector<Option> &_opts)
{
  std::string arg;
  while (!_argv.empty()) {
    arg = _argv.back();

    if (arg.size() > 1 && arg[0] == '-') {
      if (arg.size() > 2 && arg[1] == '-') {
        // is long arg

        size_t pos = arg.find('=');
        if (pos != std::string::npos) {
          // a value is assigned

          for (const auto &opt : _opts) {
            if (arg.compare(2, pos - 2, opt.longName) == 0) {
              PRINT(arg << " has been parsed.");
              opt.func(arg.substr(pos + 1));
              goto skip;
            }
          }

          ERR(arg << " is not an option.");
          exit(4); // not a long name
        } else {
          // no assigned value

          for (const auto &opt : _opts) {
            if (arg.compare(2, arg.size() - 2, opt.longName) == 0) {
              if (opt.hasArg) {
                _argv.pop_back();
                if (_argv.empty()) exit(6); // need arg

                PRINT(arg << " has been parsed.");
                opt.func(_argv.back());
              } else {
                PRINT(arg << " has been parsed.");
                opt.func("");
              }

              goto skip;
            }
          }

          ERR(arg << " is not an option.");
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
                  _argv.pop_back();
                  if (_argv.empty()) exit(2); // need arg

                  PRINT(arg << " has been parsed.");
                  opt.func(_argv.back());
                } else {
                  PRINT(arg << " has been parsed.");
                  opt.func(arg.substr(i));
                }
                goto skip;
              } else {
                PRINT(arg << " has been parsed.");
                opt.func("");
                goto next;
              }
            }
          }

          ERR(arg << " is not an option.");
          exit(1); // not a short name
        next:;
        }
      }
    } else {

      ERR(arg << " is not an option.");
      exit(3); // the fuck is this
    };

  skip:
    _argv.pop_back();
  }
}

void Parser::parse(std::vector<Option> _opts)
{
  std::move(this->optv_.begin(), this->optv_.end(), std::back_inserter(_opts));
  this->parse(this->post_, _opts);

  return;
}
