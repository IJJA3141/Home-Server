#ifndef PARAMS
#define PARAMS

#include <iostream>
#include <stdlib.h>
#include <string.h>

void parse(int _argc, char *_argv[], const char *&_pHttpPort,
           const char *&_pHttpsPort, const char *&_pKeyPath,
           const char *&_pCertPath, bool &_save, bool &_reset,
           const char *&_pSettingsPath);

#endif // !PARAMS
