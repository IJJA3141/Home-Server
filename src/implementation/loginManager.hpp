#pragma once

#include <array>
#include <filesystem>
#include <string>
#include <unistd.h>
#include <uuid/uuid.h>

struct User {
  const uuid_t uuid; // 16 byte
  const std::string name;
  const std::string password;

  User();
  ~User();
};

class LoginManger
{
public:
  LoginManger(std::filesystem::path _path);

  // Returns nullptr if there is no matching uuid
  User *get_user(const uuid_t _uuid);
  // User create_user(); // for later
  // void delete_user(); // for later

private:
  const std::filesystem::path path_;
  std::array<User*, 10> users_; // has the ownership
};
