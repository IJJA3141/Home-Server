#include "../log.hpp"
#include "auth.hpp"

#include <filesystem>
#include <fstream>
#include <openssl/sha.h>
#include <ostream>
#include <string>

#ifndef SESSION_TIME_FMT
#define SESSION_TIME_FMT "%D:%T"
#endif

template class auth_agent<session_cache_size>;

std::ostream& operator<<(std::ostream& _ostream, const session& _session)
{
  char uuid[37];
  uuid_unparse(_session.uuid, uuid);
  return _ostream << uuid << std::format("{:" SESSION_TIME_FMT "}", _session.valid_until) << _session.user;
}

template <size_t _N>
auth_agent<_N>::auth_agent(const std::filesystem::path _data_dir, const std::chrono::seconds _ttl)
    : read_path_(_data_dir / "session.db"), write_path_(_data_dir / ".session.db"), user_dir_(_data_dir / "users/"),
      ttl_(_ttl)
{
  assert(std::filesystem::is_directory(_data_dir), "'_data_dir' should be a directory");

  // initialize cache
  unsigned char null[37];
  this->fetch(null);

  return;
}

template <size_t _N> size_t auth_agent<_N>::hash_uuid(const uuid_t& _uuid) const noexcept { return _uuid[0] % _N; }

template <size_t _N>
void auth_agent<_N>::save_password_hash(const std::string_view _user, const std::string& _password) const
{
  auth_agent::user_sanitization(_user);

  // compute hash
  unsigned char hash[SHA256_DIGEST_LENGTH];
  SHA256((unsigned char*)_password.c_str(), _password.size(), hash);

  // write hash
  std::ofstream out(this->user_dir_ / _user / ".password", std::ios::out | std::ios::trunc);
  assert(out.is_open(), ""); // TODO msg
  out.write((char*)hash, SHA256_DIGEST_LENGTH);
  out << std::endl;
  out.close();
  assert(!out.fail(), ""); // TODO msg

  return;
}

template <size_t _N>
bool auth_agent<_N>::invalidate_password(const std::string_view _user, const std::string& _password) const
{
  auth_agent::user_sanitization(_user);

  // compute hash
  unsigned char hash[SHA256_DIGEST_LENGTH];
  SHA256((unsigned char*)_password.c_str(), _password.size(), hash);

  // get/check path to password
  std::filesystem::path path = this->user_dir_ / _user / ".password";
  assert(std::filesystem::exists(path), ""); // TODO msg

  // load stored password hash
  std::ifstream in(path);
  char stored_hash[SHA256_DIGEST_LENGTH];
  in.read(stored_hash, SHA256_DIGEST_LENGTH);

  // compare
  for (size_t i = 0; i < SHA256_DIGEST_LENGTH; ++i)
    if (hash[i] != stored_hash[i]) return true;

  return false;
}

template <size_t _N> const session auth_agent<_N>::generate(const std::string _user)
{
  auth_agent::user_sanitization(_user);

  // generate session
  session session = {std::chrono::system_clock::now() + this->ttl_, _user};
  uuid_generate(session.uuid);

  // store session in file
  std::ofstream cache(this->read_path_, std::ios::app);
  cache << session << "\n";
  cache.close();
  assert(!cache.fail(), "could not save generated session.");

  // store session in cache
  this->sessions_[this->hash_uuid(session.uuid)] = session;

  return session;
}

template <size_t _N> int auth_agent<_N>::fetch(const uuid_t _uuid)
{
  std::chrono::time_point now = std::chrono::system_clock::now();

  // search for match in cache
  for (int i = 0; i < _N; ++i)
    if (now < this->sessions_[i].valid_until && !uuid_compare(_uuid, this->sessions_[i].uuid)) return i;

  // open read and write files
  std::ofstream out(this->write_path_, std::ios::out | std::ios::trunc);
  std::ifstream in(this->read_path_);
  assert(in.is_open() && out.is_open());

  session session;
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

  std::filesystem::rename(this->write_path_, this->read_path_);

  return index;
}

template <size_t _N> const session auth_agent<_N>::operator[](const size_t _index) const
{
  assert(_index < _N, "Assertion '_index < _N' failed.");
  return this->sessions_[_index];
}

// TODO bether
template <size_t _N> void auth_agent<_N>::user_sanitization(const std::string_view _user)
{
  assert(!_user.empty(), "empty user name.");
  assert(_user.find("\n") == _user.npos, "\\n in user name.");

  return;
}
