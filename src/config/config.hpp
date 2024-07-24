#include <functional>
#include <string>
#include <utility>
#include <vector>

class Parser
{
private:
  std::vector<std::string> args_;

public:
  Parser(int _argc, char *_argv[], std::vector<std::string> _keys);

  void parse(std::pair<std::string, std::string> _matches, std::function<void(void)> _lambda);
  void parse(std::pair<std::string, std::string> _matches, std::function<void(int)> _lambda);
  void parse(std::pair<std::string, std::string> _matches,
             std::function<void(std::string)> _lambda);
};

struct Config {
  bool verbose = false;

  int httpPort = 80;
  int httpsPort = 443;

  std::string key = "./data/cert/key.pem";
  std::string cert = "./data/cert/cert.pem";
  std::string configPath = "./data/cert/cert.pem";

  void save();
  void load();
};
