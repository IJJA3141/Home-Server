#include "./params.hpp"
#include <cstdlib>

Params::Params(int _argc, char *_argv[]) {
  std::cout << "\n___________________________\nArguments:" << std::endl;

  bool save = true;
  this->reset_ = false;

#if DEBUG
  std::cout << "Loading..." << std::endl;
#endif

  this->Load_();

#if DEBUG
  std::cout << "Parsing..." << std::endl;
#endif

  for (int i = 1; i < _argc; i++) {
    if (strcmp("-?", _argv[i]) == 0 || strcmp("--help", _argv[i]) == 0) {
      std::cout << "usage: server  [<options>] [<arguments]\r\n"
                   "The following options are available:\r\n"
                   "-?, --help                                  Shows "
                   "available options\r\n"
                   "-n, --no-save                               Disable "
                   "options saving\r\n"
                   "-r, --reset                                 Reset to "
                   "default options, can be used with -n\r\n"
                   "                                            Http server "
                   "port\r\n"
                   "                                            Https server "
                   "port\r\n"
                   "                                            Certificate "
                   "key path\r\n"
                   "                                            Certificate "
                   "path\r\n"
                   "-h, --http-port <port>                      Change port "
                   "on witch http server will listen\r\n"
                   "-s, --https-port <port>                     Change port "
                   "on witch https server will listen\r\n"
                   "-c, --cert-path <path to certificate>       Change "
                   "certificate path\r\n"
                   "-k, --key-path <path to certificate key>    Change "
                   "certificate key path\r\n"
                << std::endl;
      exit(110);

    } else if (strcmp("-c", _argv[i]) == 0 ||
               strcmp("--cert-path", _argv[i]) == 0) {
      if (++i < _argc) {
        this->cert = _argv[i];
        std::cout << "The path to the certificate file has been set to: "
                  << this->cert << std::endl;
      } else {
        std::cout << "--cert-path command takes only one path to the "
                     "certificate file"
                  << std::endl;
        exit(111);
      }
    } else if (strcmp("-k", _argv[i]) == 0 ||
               strcmp("--key-path", _argv[i]) == 0) {
      if (++i < _argc) {
        this->key = _argv[i];
        std::cout << "The path to the certificate key file has been set to: "
                  << this->key << std::endl;
      } else {
        std::cout << "--key-path command takes only one path to the "
                     "certificate key file"
                  << std::endl;
        exit(111);
      }
    } else if (strcmp("-h", _argv[i]) == 0 ||
               strcmp("--http-port", _argv[i]) == 0) {
      if (++i < _argc) {
        this->http = _argv[i];
        std::cout << "Http port has been set to: " << this->http << std::endl;
      } else {
        std::cout << "--http-port commad takes only one port address"
                  << std::endl;
        exit(111);
      }
    } else if (strcmp("-s", _argv[i]) == 0 ||
               strcmp("--https-port", _argv[i]) == 0) {
      if (++i < _argc) {
        this->https = _argv[i];
        std::cout << "Https port had been set to: " << this->https << std::endl;
      } else {
        std::cout << "--https-port commad takes only one port address"
                  << std::endl;
        exit(110);
      }
    } else if (strcmp("-n", _argv[i]) == 0 ||
               strcmp("--no-save", _argv[i]) == 0) {
      save = false;
      std::cout << "Current parametters won't be saved" << std::endl;
    } else if (strcmp("-r", _argv[i]) == 0 ||
               strcmp("--reset", _argv[i]) == 0) {
      this->reset_ = true;
      std::cout << "Parametters have been set to default" << std::endl;
    } else {
      std::cout << _argv[i]
                << " is not a valid command\nUse -h for more informations"
                << std::endl;
      exit(111);
    }
  }

  if (this->reset_) {
#if DEBUG
    std::cout << "Reseting..." << std::endl;
#endif
    this->http = "80";
    this->https = "443";
    this->key = "./data/cert/key.pem";
    this->cert = "./data/cert/cert.pem";
  }

  if (save)
    this->Save_();

  return;
}

void Params::Load_() {
#if DEBUG
  std::cout << "Loading..." << std::endl;
#endif

  std::fstream stream;
  stream.open("./data/settings.txt", std::ios::in);

  if (stream.is_open()) {
    std::getline(stream, this->http);
    std::getline(stream, this->https);
    std::getline(stream, this->key);
    std::getline(stream, this->cert);

    stream.close();
  } else {
    std::cout << "Falied to load settings at ./data/settings.txt\nPlease be "
                 "shure to have settings.txt file in the data "
                 "directory\nServers will run with default options"
              << std::endl;
    this->reset_ = true;
  }

  return;
}

void Params::Save_() {
#if DEBUG
  std::cout << "Saving..." << std::endl;
#endif

  std::fstream stream;
  stream.open("./data/settings.txt", std::ios::out);

  if (stream.is_open()) {
    stream.clear();

    stream << this->http << "\n"
           << this->https << "\n"
           << this->key << "\n"
           << this->cert << std::endl;

    stream.close();
  } else
    std::cout << "Failed to save settings at ./data/settings.txt\nPlease be "
                 "shure to have settings.txt file in the data directory"
              << std::endl;

  return;
}
