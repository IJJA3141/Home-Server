#include "./settings.hpp"

void load(std::string *_pHttpPort, std::string *_pHttpsPort,
          std::string *_pCertPath, std::string *_pKeyPath) {
  std::fstream stream;
  stream.open("./data/settings.txt", std::ios::in);
  if (stream.is_open()) {
    std::getline(stream, *_pHttpPort);
    std::getline(stream, *_pHttpsPort);
    std::getline(stream, *_pCertPath);
    std::getline(stream, *_pKeyPath);

    stream.close();
  }
  return;
}

void save(std::string *_pHttpPort, std::string *_pHttpsPort,
          std::string *_pCertPath, std::string *_pKeyPath) {
  std::fstream stream;
  stream.open("./data/settings.txt", std::ios::out);
  if (stream.is_open()) {
    stream.clear();
    stream << _pHttpPort << "\n"
           << _pHttpsPort << "\n"
           << _pCertPath << "\n"
           << _pKeyPath << "\n";

    stream.close();
  }

  return;
}
