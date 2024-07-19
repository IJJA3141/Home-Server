#include <fstream>
#include <iostream>
#include <string>

struct Config {
  bool verbose = false;
  int httpPort = 80;
  int httpsPort = 443;
  std::string keyPath = "./data/cert/key.pem";
  std::string certPath = "./data/cert/cert.pem";
};

void loadConfig(const std::string _configPath, Config &_config)
{
  std::fstream stream;
  stream.open(_configPath, std::ios::in);

  if (stream.is_open()) {
    std::string str;
    while (std::getline(stream, str)) {
      if (str.find("http="))
        _config.httpPort = std::atoi(str.erase(0, 5).c_str());
      else if (str.find("https="))
        _config.httpPort = std::atoi(str.erase(0, 6).c_str());
      else if (str.find("cert="))
        _config.httpPort = std::atoi(str.erase(0, 5).c_str());
      else if (str.find("key="))
        _config.httpPort = std::atoi(str.erase(0, 4).c_str());
      else
        std::cout << "??" << std::endl;
    };

    stream.close();
  } else {
    std::cout << "err" << std::endl;
  }

  return;
};

void saveConfig(Config _config)
{
  std::fstream stream;
  stream.open("./data/settings.txt", std::ios::out);

  if (stream.is_open()) {
    stream.clear();

    stream << "http=" << _config.httpPort << "\nhttps=" << _config.httpsPort
           << "\ncert=" << _config.certPath << "\nkey=" << _config.certPath << std::endl;

    stream.close();
  } else {
    std::cout << "" << std::endl;
  }

  return;
}

int main(int _argc, char *_argv[])
{
  bool printHelp = false;
  bool save = false;
  bool reset = false;
  std::string configPath = "./data/settings.txt";

  Config conf;

  // index at one to skip path to exec
  for (int i = 1; i < _argc; i++) {
    std::string str(_argv[i]);

    if (str == "--help" || str == "-?") {
      printHelp = true;
      break;
    } else if (str == "--config" || str == "-c") {
      if (++i < _argc) {
        configPath = std::string(_argv[i]);
      } else {
        std::cout << "config need path" << std::endl;
        return 1;
      }
    } else if (str == "--cert-path") {
      if (++i < _argc) {
        conf.certPath = std::string(_argv[i]);
      } else {
        std::cout << "cert need path" << std::endl;
        return 1;
      }
    } else if (str == "--key-path") {
      if (++i < _argc) {
        conf.keyPath = std::string(_argv[i]);
      } else {
        std::cout << "key need path" << std::endl;
        return 1;
      }
    } else if (str == "--http-port" || str == "-h") {
      if (++i < _argc) {
        conf.httpPort = std::atoi(_argv[i]);
      } else {
        std::cout << "http need value" << std::endl;
        return 1;
      }
    } else if (str == "--https-port" || str == "-p") {
      if (++i < _argc) {
        conf.httpsPort = std::atoi(_argv[i]);
      } else {
        std::cout << "https need value" << std::endl;
        return 1;
      }
    } else if (str == "--save" || str == "-s")
      save = true;
    else if (str == "--verbose" || str == "-v")
      conf.verbose = true;
    else if (str == "--default" || str == "-d")
      reset = true;
    else {
      std::cout << "?" << std::endl;
      return 1;
    }
  }

  if (printHelp) {
    std::cout << "o_o" << std::endl;
    return 0;
  }

  if (reset)
    conf = Config();
  else
    loadConfig(configPath, conf);

  if (save) saveConfig(conf);

  return 0;
}
