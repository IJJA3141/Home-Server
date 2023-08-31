#include "./params.hpp"
#include <cstring>

void parse(int _argc, char *_argv[], const char *&_pHttpPort,
           const char *&_pHttpsPort, const char *&_pKeyPath,
           const char *&_pCertPath, bool &_save, bool &_reset,
           const char *&_pSettingsPath) {
  for (int i = 1; i < _argc; i++) {
    if (strcmp("-?", _argv[i]) == 0 || strcmp("--help", _argv[i]) == 0) {

      std::cout
          << "usage: server  [<options>] [<arguments]\r\n"
             "The following options are available:\r\n"
             "-?, --help                                          Shows "
             "available options\r\n"
             "-n, --no-save                                       Disable "
             "options saving\r\n"
             "-r, --reset                                         Reset to "
             "default options, can be used with -n\r\n"
             "-i, --import <path to txt file of new settings>     Import "
             "settings from txt file at specified path\r\n"
             "                                                    /!\\ "
             "Important the txt file should folow this format:\r\n"
             "                                                    Http server "
             "port\r\n"
             "                                                    Https server "
             "port\r\n"
             "                                                    Certificate "
             "key path\r\n"
             "                                                    Certificate "
             "path\r\n"
             "-h, --http-port <port>                              Change port "
             "on witch http server will listen\r\n"
             "-s, --https-port <port>                             Change port "
             "on witch https server will listen\r\n"
             "-c, --cert-path <path to certificate>               Change "
             "certificate path\r\n"
             "-k, --key-path <path to certificate key>            Change "
             "certificate key path\r\n"
          << std::endl;
      abort();

    } else if (strcmp("-c", _argv[i]) == 0 ||
               strcmp("--cert-path", _argv[i]) == 0) {
      if (++i < _argc) {
        _pCertPath = _argv[i];

        // debug only
        //
        std::cout << "Path to certificate: " << _pCertPath << std::endl;

      } else {
        std::cout << "Too many arguments\r\n" << std::endl;
        abort();
      }
    } else if (strcmp("-k", _argv[i]) == 0 ||
               strcmp("--key-path", _argv[i]) == 0) {
      if (++i < _argc) {
        _pKeyPath = _argv[i];

        // debug only
        //
        std::cout << "Path to certificate key: " << _pKeyPath << std::endl;

      } else {
        std::cout << "Too many arguments\r\n" << std::endl;
        abort();
      }
    } else if (strcmp("-h", _argv[i]) == 0 ||
               strcmp("--http-port", _argv[i]) == 0) {
      if (++i < _argc) {
        _pHttpPort = _argv[i];

        // debug only
        //
        std::cout << "Http port: " << _pHttpPort << std::endl;

      } else {
        std::cout << "Too many arguments\r\n" << std::endl;
        abort();
      }
    } else if (strcmp("-s", _argv[i]) == 0 ||
               strcmp("--https-port", _argv[i]) == 0) {
      if (++i < _argc) {
        _pHttpsPort = _argv[i];

        // debug only
        //
        std::cout << "Https port: " << _pHttpsPort << std::endl;

      } else {
        std::cout << "Too many arguments\r\n" << std::endl;
        abort();
      }
    } else if (strcmp("-n", _argv[i]) == 0 ||
               strcmp("--no-save", _argv[i]) == 0) {
      _save = false;

      // debug only
      //
      std::cout << "No save" << std::endl;

    } else if (strcmp("-r", _argv[i]) == 0 ||
               strcmp("--reset", _argv[i]) == 0) {
      _reset = true;

      // debug only
      //
      std::cout << "Reset" << std::endl;

    } else if (strcmp("-i", _argv[i]) == 0 ||
               strcmp("--import", _argv[i]) == 0) {
      if (++i < _argc) {
        _pSettingsPath = _argv[i];

        // debug only
        //
        std::cout << "Settings path:" << _pSettingsPath << std::endl;
        char buffer[sizeof(_argv[0]) + sizeof(_pSettingsPath)];
  strcpy(buffer, _argv[0]);
  strcpy(buffer + sizeof(_argv[0]), _pSettingsPath);
  // debug only
  //
  std::cout << "New path: " << buffer << std::endl;

  _pSettingsPath = buffer;

  // debug only
  //
  std::cout << "New new path: " << _pSettingsPath << std::endl;


      } else {
        std::cout << "Too many arguments\r\n" << std::endl;
        abort();
      }
    } else {
      std::cout << _argv[i]
                << " is not a valid option\r\nUse -h for more informations"
                << std::endl;
      abort();
    }
  }

  }
