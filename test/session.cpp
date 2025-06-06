#include "../src/io/auth.hpp"
#include <filesystem>

int session(int argc, char* argv[]) {
  auto path = std::filesystem::current_path();
  path.append("data");
  path.append("sessions.db");
  session_cache cache(path);

  cache.generate("user 1");
  cache.generate("user 2");
  cache.generate("user 3");

  return 0;
}
