#include "../server/reqres.hpp"
#include "test.hpp"
#include <string>

void test::stream()
{
  Stream stream("1 2\n3\r\n4  \n\n\r\n\r\n");
  std::string str;
  bool failed = false;

  failed = !(stream >> str);
  failed = (str != "1") || failed;

  failed = !(stream >> str);
  failed = (str != "2") || failed;

  failed = !(stream >> str);
  failed = (str != "3") || failed;

  failed = !(stream >> str);
  failed = (str != "") || failed;

  failed = !(stream >> str);
  failed = (str != "") || failed;

  failed = !(stream >> str);
  failed = (str != "") || failed;

  if (failed) {
    ERR("stream test failed");
    return;
  }

  LOG("stream test passed");
  return;
};
