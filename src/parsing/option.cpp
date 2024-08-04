#include "parsing.hpp"

Option::Option(int _shortName, std::string _longName, std::function<void(std::string)> _func)
    : func(_func), longName(_longName), shortName(_shortName), hasArg(true){};

Option::Option(int _shortName, std::string _longName, std::function<void(void)> _func)
    : longName(_longName), shortName(_shortName), hasArg(false),
      func([_func](std::string) { _func(); }){};

Option::Option(int _shortName, std::string _longName, std::function<void(int)> _func)
    : longName(_longName), shortName(_shortName), hasArg(true),
      func([_func](std::string _) { _func(atoi(_.c_str())); }){};
