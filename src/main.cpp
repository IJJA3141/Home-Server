#include <any>
#include <functional>
#include <map>
#include <string>

struct Config {
};

struct AFn {
  AFn(std::function<void(std::string)> _lambda);
  AFn(std::function<void(void)> _lambda);
  AFn(std::function<void(int)> _lambda);

private:
  enum Type {
    STRING,
    VOID,
    INT,
  };

  Type type;

  std::function<void(void)> voidFn_;
  std::function<void(int)> intFn_;
  std::function<void(std::string)> stringFn_;
};

AFn::AFn(std::function<void(void)> _lambda)
{
  this->type = AFn::Type::VOID;
  this->voidFn_ = _lambda;

  return;
};

AFn::AFn(std::function<void(std::string)> _lambda)
{
  this->type = AFn::Type::STRING;
  this->stringFn_ = _lambda;

  return;
};

AFn::AFn(std::function<void(int)> _lambda)
{
  this->type = AFn::Type::INT;
  this->intFn_ = _lambda;

  return;
};

class Parser
{
private:
  std::map<std::string, AFn *> map_;

public:
  Parser();

  template <typename... Args> void add(std::function<void(std::string)> _lambda, Args... args);
  template <typename... Args> void add(std::function<void(void)> _lambda, Args... args);
  template <typename... Args> void add(std::function<void(int)> _lambda, Args... args);
  Config parse();
};

Parser::Parser(){};

template <typename... Args> void Parser::add(std::function<void(void)> _lambda, Args... _keys)
{
  AFn *ptr = new AFn(_lambda);

  for (const auto key : {_keys...})
    this->map_[key] = ptr;

  return;
};

template <typename... Args> void Parser::add(std::function<void(int)> _lambda, Args... _keys)
{
  AFn *ptr = new AFn(_lambda);

  for (const auto key : {_keys...})
    this->map_[key] = ptr;

  return;
};

template <typename... Args>
void Parser::add(std::function<void(std::string)> _lambda, Args... _keys)
{
  AFn *ptr = new AFn(_lambda);

  for (const auto key : {_keys...})
    this->map_[key] = ptr;

  return;
};

int main(int argc, char *argv[])
{
  Parser par;

  par.add([]() {}, "", "", "", "");
  par.add([](int) {}, "", "", "", "");
  par.add([](std::string) {}, "", "", "", "");

  return 0;
}
