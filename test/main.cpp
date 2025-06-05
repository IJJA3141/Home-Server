#include "test.hpp"

int main(int _argc, char *_argv[]) {
  LOG("testing...")
  __test_split(_argc, _argv);
  LOG("__test_split passed")
  __test_cookies(_argc, _argv);
  LOG("__test_cookies passed")
  __test_request(_argc, _argv);
  LOG("__test_request passed")

  return 0;
}
