#include "./params/params.hpp"
#include "./settings/settings.hpp"

int main(int _argc, char *_argv[]) {

  const char *pHttpPort;
  const char *pHttpsPort;
  const char *pKeyPaht;
  const char *pCertPath;
  bool save = true;
  bool reset = false;

  parse(_argc, _argv, pHttpPort, pHttpsPort, pKeyPaht, pCertPath, save, reset);

  return 0;
}
