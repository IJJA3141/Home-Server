#include "options.hpp"
#include <algorithm>

int getSubCommand(const std::vector<std::string> &_scmds, const std::vector<std::string> &_argv)
{
  for (const auto &arg : _argv)
    for (size_t i = 0; i < _scmds.size(); i++)
      if (_scmds[i] == arg) return i;

  return -1;
}

Parser::Parser(std::vector<std::string> _scmds, std::vector<std::string> _argv)
{
  int index = getSubCommand(_scmds, _argv);

  if (index != -1) {
    this->pre_ = std::vector<std::string>(_argv.begin(), _argv.begin() + index);
    std::reverse(this->pre_.begin(), this->pre_.end());

    this->post_ = std::vector<std::string>(_argv.begin() + index + 1, _argv.end());
    this->scmd_ = _argv[index];
  } else {
    this->pre_ = _argv;
    this->scmd_ = "!NSCM";
  }

  return;
}

template <size_t S> void Parser::parse(Option _options[S])
{
  while (!this->pre_.empty()) {
    if (this->pre_.back().size() > 2 && this->pre_.back()[0] == '-') {
      if (this->pre_.back().size() > 3 && this->pre_.back()[1] == '-') {
        // is a long option

        size_t eq = this->pre_.back().find('=');

        if (eq != std::string::npos) {
          // a value has been assigned

          std::string key(this->pre_.back().begin() + 2, this->pre_.back().begin() + eq);
          std::string val(this->pre_.back().begin() + eq + 1, this->pre_.back().end());

          for (Option const &opt : _options) {
            if (key == opt.longName) {
              if (opt.type == Option::Type::NOARG) exit(1); // err

              opt.func(val);
              this->pre_.pop_back();
              return;
            }

            // to do
            exit(-1);
          }
        } else {
          // no value assigned
          for (Option const &opt : _options) {
            if (opt.longName.compare(2, this->pre_.back().size() - 2, this->pre_.back()) == 0) {
              this->pre_.pop_back();

              if (opt.type == Option::Type::NOARG) {
                // no need for args

                opt.func("");
                return;
              } else {
                // check if arg

                if (this->pre_.back()[0] == '-') exit(2); // err

                opt.func(this->pre_.back());
                this->pre_.pop_back();
                return;
              }
            }
          }

          // to do
          exit(-2);
        }
      } else {
        // is a short option

        // one to skip -
        for (size_t i = 1; i < this->pre_.back().size(); i++) {
          for (const Option &opt : _options) {
            if (this->pre_.back()[i] == opt.shortName) {
              if (opt.type == Option::Type::NOARG) {
                this->pre_.back().erase(i, 1);
                opt.func("");
                return;
              } else {
              }
            };
          }
        }
      }
    } else {
      // is not an option
    }
  }
}
