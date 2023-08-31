#include "./settings.hpp"
#include <cstring>

void load(const char *&_pHttpPort, const char *&_pHttpsPort,
          const char *&_pKeyPath, const char *&_pCertPath,
          const char *_pSettingsPath) {
  std::fstream stream;
  stream.open(_pSettingsPath, std::ios::in);

  if (stream.is_open()) {
    std::string str;
 
    std::getline(stream, str);
    strcpy

    std::getline(stream, str1);
    _pHttpsPort = str1.c_str();
    std::getline(stream, str2);
    _pKeyPath = str2.c_str();
    std::getline(stream, str3);
    _pCertPath = str3.c_str();

    // debug only
    //
    std::cout << "Http: " << _pHttpPort << "\r\nHttps: " << _pHttpsPort
              << "\r\nKey: " << _pKeyPath << "\r\nCert: " << _pCertPath
              << std::endl;

    stream.close();
  } else
    std::cout << "Failed to open " << _pSettingsPath
              << "\r\nStarting with default config" << std::endl;

  return;
}

void save(const char *_pHttpPort, const char *_pHttpsPort,
          const char *_pKeyPath, const char *_pCertPath,
          const char *_pSettingsPath) {
  std::fstream stream;
  stream.open(_pSettingsPath, std::ios::out);
  if (stream.is_open()) {
    // debug only
    //
    std::cout << "Http: " << _pHttpPort << "\r\nHttps: " << _pHttpsPort
              << "\r\nKey: " << _pKeyPath << "\r\nCert: " << _pCertPath
              << std::endl;

    stream.clear();
    stream << _pHttpPort << "\n"
           << _pHttpsPort << "\n"
           << _pCertPath << "\n"
           << _pKeyPath << "\n";

    stream.close();
  } else
    std::cout << "Failed to save config" << std::endl;

  return;
}
