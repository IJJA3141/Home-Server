#include <cassert>
#include <iterator>
#include <utility>

#include "../log.hpp"
#include "parsing.hpp"

Parser::Parser(int _argc, char *_argv[], std::vector<std::string> _scmds) : scmd("")
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

    this->argv_.push_back(_argv[i]);
  }

end:
  for (; i < _argc; i++)
    this->subArgv_.push_back(_argv[i]);

  PRINT("pre scmd");
  PRINTV(this->argv_);
  PRINT("scmd");
  PRINT(this->scmd);
  PRINT("post scmd");
  PRINTV(this->subArgv_);

  std::reverse(this->argv_.begin(), this->argv_.end());
  std::reverse(this->subArgv_.begin(), this->subArgv_.end());

  return;
}

void Parser::add(std::vector<Option> _optv)
{
  std::move(_optv.begin(), _optv.end(), std::back_inserter(this->optv_));
  return;
}

void Parser::parse()
{
  this->parse(this->argv_);
  return;
}

void Parser::subParse()
{
  this->parse(this->subArgv_);
  return;
}

void Parser::parse(std::vector<std::string> &_argv)
{
  assert(!this->optv_.empty());

  while (!_argv.empty()) {
    this->arg_ = _argv.back();

    if (this->arg_.size() > 1 && this->arg_[0] == '-') {
      if (this->arg_.size() > 2 && this->arg_[1] == '-')
        this->longArg(_argv);
      else
        this->shortArg(_argv);

    } else {
      ERR(this->arg_ << " is not an option.");
      exit(3); // the fuck is this
    };

    _argv.pop_back();
  }

  return;
}

void Parser::longArg(std::vector<std::string> &_argv)
{
  size_t pos = this->arg_.find('=');
  if (pos != std::string::npos) {
    // a value is assigned

    for (const auto &opt : this->optv_) {
      if (this->arg_.compare(2, pos - 2, opt.longName) == 0) {
        PRINT(this->arg_ << " has been parsed.");
        opt.func(this->arg_.substr(pos + 1));

        return;
      }
    }

    ERR(this->arg_ << " is not an option.");
    exit(4); // not a long name
  } else {
    // no assigned value

    for (const auto &opt : this->optv_) {
      if (this->arg_.compare(2, this->arg_.size() - 2, opt.longName) == 0) {
        if (opt.hasArg) {
          _argv.pop_back();
          if (_argv.empty()) exit(6); // need arg

          PRINT(this->arg_ << " has been parsed.");
          opt.func(_argv.back());
        } else {
          PRINT(this->arg_ << " has been parsed.");
          opt.func("");
        }

        return;
      }
    }

    ERR(this->arg_ << " is not an option.");
    exit(5); // not a long name
  }

  assert(false); // unreachable
};

void Parser::shortArg(std::vector<std::string> &_argv)
{
  // start at one to skip -
  for (size_t i = 1; i < this->arg_.size(); i++) {
    for (const auto &opt : optv_) {
      if (this->arg_[i] == opt.shortName) {
        if (opt.hasArg) {
          if (++i >= this->arg_.size()) {
            _argv.pop_back();
            if (_argv.empty()) exit(2); // need arg

            PRINT(this->arg_ << " has been parsed.");
            opt.func(_argv.back());
          } else {
            PRINT(this->arg_ << " has been parsed.");
            opt.func(this->arg_.substr(i));
          }
          return;
        } else {
          PRINT(this->arg_ << " has been parsed.");
          opt.func("");
          goto next;
        }
      }
    }

    ERR(this->arg_ << " is not an option.");
    exit(1); // not a short name

  next:;
  }
}
