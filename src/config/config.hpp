#include <functional>
#include <string>
#include <vector>

class Parser
{
private:
  std::vector<std::string> _args;

public:
  Parser(int _argc, char *_argv[]);

  void parse(std::pair<std::string, std::string> _matches, std::function<void(int)> _lambda);
  void parse(std::pair<std::string, std::string> _matches, std::function<void(void)> _lambda);
  void parse(std::pair<std::string, std::string> _matches, std::function<void(bool)> _lambda);
  void parse(std::pair<std::string, std::string> _matches, std::function<void(std::string)> _lambda);
};
