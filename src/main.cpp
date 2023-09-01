#include "./params/params.hpp"
#include "./settings/settings.hpp"

int main(int _argc, char *_argv[]) {
  const char *pHttpPort;
  const char *pHttpsPort;
  const char *pKeyPath;
  const char *pCertPath;
  const char *pSettingsPath = "./data/settings.txt";
  bool saveState = true;
  bool reset = false;

  char buffer[sizeof(_argv[0]) + strlen(pSettingsPath)];
  strcpy(buffer, _argv[0]);
  strcpy(buffer + sizeof(_argv[0]), pSettingsPath);

  load(pHttpPort, pHttpsPort, pKeyPath, pCertPath, pSettingsPath);
  parse(_argc, _argv, pHttpPort, pHttpsPort, pKeyPath, pCertPath, saveState,
        reset, pSettingsPath);

  if (reset) {
    pHttpPort = "80";
    pHttpsPort = "443";
    pKeyPath = "./cert/key.pem";
    pCertPath = "./cert/cert.pem";
  }

  save(pHttpPort, pHttpsPort, pKeyPath, pCertPath, pSettingsPath);

  return 0;
}
