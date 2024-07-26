#include <iostream>

#include "config.hpp"

int main(int _argc, char *_argv[])
{
#ifdef DEBUG
  return 0;
#else
  return 1;
#endif // DEBUG

  parse(_argc, _argv,
        {
            {"--help", [] { std::cout << "help me !" << std::endl; }},
        });

  return 0;
}
