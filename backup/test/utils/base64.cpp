#include "../../src/utils/base64.hpp"
#include "../test.hpp"

using namespace test;

int utils_base64(int argc, char* argv[])
{
  int _flag = 0, _i = 0, _j = 0;
  bool _exception_caught = false;

  SECTION("base64 decode")
  {
    REQUIRE("decode empty string", { assert_equal<std::string>(base64_decode(""), ""); });
    REQUIRE("decode single byte", { assert_equal<std::string>(base64_decode("Zg=="), "f"); });
    REQUIRE("decode two bytes", { assert_equal<std::string>(base64_decode("Zm8="), "fo"); });
    REQUIRE("decode three bytes", { assert_equal<std::string>(base64_decode("Zm9v"), "foo"); });
    REQUIRE("decode multiple groups", { assert_equal<std::string>(base64_decode("Zm9vYmFy"), "foobar"); });
    REQUIRE("decode url safe alphabet", { assert_equal<std::string>(base64_decode("++//"), "\xfb\xef\xff"); });
    REQUIRE("decode padded string", { assert_equal<std::string>(base64_decode("SGVsbG8="), "Hello"); });
    REQUIRE("decode sentence", { assert_equal<std::string>(base64_decode("SGVsbG8gV29ybGQ="), "Hello World"); });
    REQUIRE_THROW("reject invalid size", std::invalid_argument, { base64_decode("abc"); });
    REQUIRE_THROW("reject invalid character", std::invalid_argument, { base64_decode("Zm$="); });
    REQUIRE_THROW("reject invalid padding character", std::invalid_argument, { base64_decode("A==="); });
  }

  ENDSECTION;

  return _flag;
}
