#ifndef SETTINGS
#define SETTINGS

#include <string>
#include <fstream>
#include <iostream>

void load(const char *&_pHttpPort, const char *&_pHttpsPort,
          const char *&_pKeyPath, const char *&_pCertPath,
          const char *_pSettingsPath);
void save(const char *_pHttpPort, const char *_pHttpsPort,
          const char *_pKeyPaht, const char *_pCertPath,
          const char *_pSettingsPath);

#endif // !SETTINGS
