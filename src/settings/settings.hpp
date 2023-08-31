#ifndef SETTINGS
#define SETTINGS

#include <fstream>
#include <iostream>
#include <string>

void load(std::string *_pHttpPort, std::string *_pHttpsPort,
          std::string *_pCertPath, std::string *_pKeyPath);
void save(std::string *_pHttpPort, std::string *_pHttpsPort,
          std::string *_pCertPath, std::string *_pKeyPath);

#endif // !SETTINGS
