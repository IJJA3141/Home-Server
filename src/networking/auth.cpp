#include "auth.hpp"
#include "../log.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <openssl/sha.h>
#include <ostream>
#include <string>

#ifndef SESSION_TIME_FMT
#define SESSION_TIME_FMT "%D:%T"
#endif

std::ostream& operator<<(std::ostream& _ostream, const Session& _session)
{
  char uuid[37];
  uuid_unparse(_session.uuid, uuid);
  return _ostream << uuid << std::format("{:" SESSION_TIME_FMT "}", _session.valid_until) << _session.user;
}

std::string& operator<<(std::string& _ostream, const Session& _session)
{
  char uuid[37];
  uuid_unparse(_session.uuid, uuid);
  return ((((_ostream += "session=") += uuid) += "; Max-Age=") +=
          std::to_string(std::chrono::duration_cast<std::chrono::seconds>(_session.valid_until -
                                                                          std::chrono::system_clock::now())
                             .count())) += "; Secure; HttpOnly; SameSite=Strict; Path=/";
}

Authenticator::Authenticator(const size_t _size, const std::filesystem::path _data_dir,
                             const std::chrono::seconds _ttl)
    : read_path(_data_dir / "session.db"), write_path(_data_dir / ".session.db"), user_dir(_data_dir / "users/"),
      ttl_(_ttl), size_(_size), sessions_(new Session[_size])
{
  assert(std::filesystem::is_directory(_data_dir), "'_data_dir' should be a directory");

  // initialize cache
  unsigned char null[37];
  this->fetch(null);

  return;
}

size_t Authenticator::hash_uuid(const uuid_t& _uuid) const noexcept { return _uuid[0] % this->size_; }

void Authenticator::save_password_hash(const std::string_view _user, const std::string& _password)
{
  Authenticator::user_sanitization(_user);

  // compute hash
  unsigned char hash[SHA256_DIGEST_LENGTH];
  SHA256((unsigned char*)_password.c_str(), _password.size(), hash);

  // write hash
  std::ofstream out(this->user_dir / _user / ".password", std::ios::out | std::ios::trunc);
  assert(out.is_open(), "couldn't open", this->user_dir / _user / ".password");
  out.write((char*)hash, SHA256_DIGEST_LENGTH);
  out << std::endl;
  out.close();
  assert(!out.fail(), "couldn't close", this->user_dir / _user / ".password");

  return;
}

bool Authenticator::invalidate_password(const std::string_view _user, const std::string& _password) const
{
  Authenticator::user_sanitization(_user);

  // compute hash
  unsigned char hash[SHA256_DIGEST_LENGTH];
  SHA256((unsigned char*)_password.c_str(), _password.size(), hash);

  // get/check path to password
  std::filesystem::path path = this->user_dir / _user / ".password";
  if (!std::filesystem::exists(path))
  {
    err("couldn't find", this->user_dir / _user / ".password");
    return true;
  }

  // load stored password hash
  std::ifstream in(path);
  unsigned char stored_hash[SHA256_DIGEST_LENGTH];
  in.read((char*)stored_hash, SHA256_DIGEST_LENGTH);

  bool constant_time = false;
  
  // compare
  for (size_t i = 0; i < SHA256_DIGEST_LENGTH; ++i)
    if (hash[i] != stored_hash[i]) constant_time = true;

  return constant_time;
}

const Session Authenticator::generate(const std::string _user)
{
  Authenticator::user_sanitization(_user);

  // generate session
  Session session = {std::chrono::system_clock::now() + this->ttl_, _user};
  uuid_generate(session.uuid);

  // store session in file
  std::ofstream cache(this->read_path, std::ios::app);
  cache << session << "\n";
  cache.close();
  assert(!cache.fail(), "could not save generated session.");

  // store session in cache
  this->sessions_[this->hash_uuid(session.uuid)] = session;

  return session;
}

int Authenticator::fetch(const uuid_t _uuid)
{
  std::chrono::time_point now = std::chrono::system_clock::now();

  // search for match in cache
  for (int i = 0; i < this->size_; ++i)
    if (now < this->sessions_[i].valid_until && !uuid_compare(_uuid, this->sessions_[i].uuid)) return i;

  // open read and write files
  std::ofstream out(this->write_path, std::ios::out | std::ios::trunc);
  std::ifstream in(this->read_path);

  assert(out.is_open(), "couldn't open write path", this->write_path);
  assert(in.is_open(), "couldn't open read path", this->read_path);

  Session session;
  char buffer[37];
  int index = -1;

  buffer[36] = '\0';

  while (in.read(buffer, 36))
  {
    // read session
    assert(!uuid_parse(buffer, session.uuid), "could not parse uuid: " + std::string(buffer));
    in >> std::chrono::parse(SESSION_TIME_FMT, session.valid_until);
    assert(!in.fail(), "could not parse life time");
    std::getline(in, session.user);
    assert(!in.fail(), "could not parse user name");

    if (now < session.valid_until)
    {
      // store session if valid
      out << session << std::endl;
      this->sessions_[this->hash_uuid(session.uuid)] = session;

      // set index to matching session
      if (!uuid_compare(_uuid, session.uuid)) index = this->hash_uuid(session.uuid);
    }
  }

  // save cache
  in.close();
  out.close();
  assert(!out.fail(), "failed to save cache");

  std::filesystem::rename(this->write_path, this->read_path);

  return index;
}

const Session Authenticator::operator[](const size_t _index) const
{
  assert(_index < this->size_, "Assertion '_index < _N' failed.");
  return this->sessions_[_index];
}

// TODO bether
void Authenticator::user_sanitization(const std::string_view _user)
{
  assert(!_user.empty(), "empty user name.");
  assert(_user.find("\n") == _user.npos, "\\n in user name.");

  return;
}

#include <sys/epoll.h>
