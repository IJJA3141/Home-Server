#include "loginManager.hpp"
#include "../log.hpp"
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <unistd.h>
#include <uuid/uuid.h>

LoginManger::LoginManger(std::filesystem::path _path) : path_(_path) { return; }

User *LoginManger::get_user(const uuid_t _uuid)
{
  for (User *user : this->users_)
    if (uuid_compare(_uuid, user->uuid) == 0) return user;

  std::ifstream file(this->path_);
  if (!file.is_open()) {
    WARN("");
    return nullptr;
  }

  std::string buffer;
  file.seekg(0, std::ios::end);
  buffer.reserve(file.tellg());
  file.seekg(0, std::ios::beg);

  buffer.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
  PRINT(buffer);

  return nullptr;
}

/* user save file template
 *
 * ---------------------------
 * |uuid|;|name|;|password|\n|
 * ---------------------------
 */
