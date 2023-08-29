#include "./params/params.hpp"
#include "./settings/settings.hpp"

#include <string>

int main(int _argc, char *_argv[]) {
  std::string a;
  std::string b;
  std::string c;
  std::string d;

  load(&a, &b, &c, &d);
  std::cout << params(_argc, _argv, &a, &b, &c, &d) << std::endl;
  save(&a, &b, &c, &d); 

  return 0;
}
