#include <functional>
#include <string>
#include <vector>

class AbstractFunction
{
  enum Type { VOID, STRING, INT };
  Type type_;

  std::function<void(std::string)> fString_;
  std::function<void(void)> fVoid_;
  std::function<void(int)> fInt_;

  AbstractFunction(std::function<void(std::string)> _func);
  AbstractFunction(std::function<void(void)> _func);
  AbstractFunction(std::function<void(int)> _func);

  void operator()();
};

AbstractFunction::AbstractFunction(std::function<void(std::string)> _func)
{
  this->type_ = AbstractFunction::STRING;
  this->fString_ = _func;

  return;
};

AbstractFunction::AbstractFunction(std::function<void(void)> _func)
{
  this->type_ = AbstractFunction::VOID;
  this->fVoid_ = _func;

  return;
};

AbstractFunction::AbstractFunction(std::function<void(int)> _func)
{
  this->type_ = AbstractFunction::INT;
  this->fInt_ = _func;

  return;
};

void AbstractFunction::operator()()
{
  switch (this->type_) {
  case AbstractFunction::VOID:
    this->fVoid_();
  case AbstractFunction::STRING:
    this->fString_("");
  case AbstractFunction::INT:
    this->fInt_(2);
  default:
    exit(-1);
  }

  return;
}

int main(int argc, char *argv[])
{
  std::vector<AbstractFunction> vec;
  
  return 0;
}
