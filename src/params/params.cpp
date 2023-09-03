#include "./params.hpp"

Params::Params(int _argc, char *_argv[]) {
  bool save = true;
  bool reset = false;

  this->load_();

  std::cout << "Parsing..." << std::endl;

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
      abort();

    } else if (strcmp("-c", _argv[i]) == 0 ||
               strcmp("--cert-path", _argv[i]) == 0) {
      if (++i < _argc) {
        this->cert = _argv[i];
      } else {
        std::cout << "Too many arguments\r\n" << std::endl;
        abort();
      }
    } else if (strcmp("-k", _argv[i]) == 0 ||
               strcmp("--key-path", _argv[i]) == 0) {
      if (++i < _argc) {
        this->key = _argv[i];
      } else {
        std::cout << "Too many arguments\r\n" << std::endl;
        abort();
      }
    } else if (strcmp("-h", _argv[i]) == 0 ||
               strcmp("--http-port", _argv[i]) == 0) {
      if (++i < _argc) {
        this->http = _argv[i];
      } else {
        std::cout << "Too many arguments\r\n" << std::endl;
        abort();
      }
    } else if (strcmp("-s", _argv[i]) == 0 ||
               strcmp("--https-port", _argv[i]) == 0) {
      if (++i < _argc) {
        this->https = _argv[i];
      } else {
        std::cout << "Too many arguments\r\n" << std::endl;
        abort();
      }
    } else if (strcmp("-n", _argv[i]) == 0 ||
               strcmp("--no-save", _argv[i]) == 0) {
      save = false;
    } else if (strcmp("-r", _argv[i]) == 0 ||
               strcmp("--reset", _argv[i]) == 0) {
      reset = true;
    } else {
      std::cout << _argv[i]
                << " is not a valid option\r\nUse -h for more informations"
                << std::endl;
      abort();
    }
  }

  if (reset) {
    std::cout << "Reseting..." << std::endl;
    this->http = "80";
    this->https = "443";
    this->key = "./data/cert/key.pem";
    this->cert = "./data/cert/cert.pem";
  } else {
  }

  if (save)
    this->save_();

  return;
}

void Params::load_() {
  std::cout << "Loading..." << std::endl;

  std::fstream stream;
  stream.open("./data/settings.txt", std::ios::in);

  if (stream.is_open()) {
    std::getline(stream, this->http);
    std::getline(stream, this->https);
    std::getline(stream, this->key);
    std::getline(stream, this->cert);

    stream.close();
  } else {
    std::cout << "Falied to load settings at " << this->path_ << std::endl;
    abort();
  }

  return;
}

void Params::save_() {
  std::cout << "Saving..." << std::endl;

  std::fstream stream;
  stream.open("./data/settings.txt", std::ios::out);

  if (stream.is_open()) {
    stream.clear();

    stream << this->http << "\n"
           << this->https << "\n"
           << this->key << "\n"
           << this->cert << std::endl;

    stream.close();
  } else {
    std::cout << "Failed to save settings" << std::endl;
    abort();
  }

  return;
}
