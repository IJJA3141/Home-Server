#include "../src/config.hpp"
#include "../src/log.hpp"
#include "../src/network/auth.hpp"

#include <filesystem>
#include <fstream>
#include <string>
#include <uuid/uuid.h>
#include <vector>

bool test_load()
{
  const auto path = std::filesystem::current_path() / "test" / "data" / "load";

  const auto data = "334dc436-d22c-487c-84ce-cf9e369d5a3206/12/30:20:38:25.149326666user 1\n"
                    "1fade928-1b0b-4cbc-8773-1e5a1484f1d206/12/30:20:38:25.149326666user 3\n"
                    "542f9292-d998-4d5e-bf00-226b68ed677d06/12/23:20:38:25.149326666user 4\n"
                    "b0a392ab-2fd3-4236-8dee-44d75efca11706/12/23:20:38:25.149326666user 5\n"
                    "54894c59-eb8f-4776-ad7e-57727162069706/12/30:20:38:25.149326666user 6\n"
                    "36738adf-332c-40f0-9dbc-c1ae9fc6df8f06/12/30:20:38:25.149326666user 7\n"
                    "08b92265-9f6e-4ab0-bc47-618f5021fc8406/12/30:20:38:25.149326666user 8\n"
                    "7eb9aba8-4959-4c66-ad4c-9bfa63edfb4f06/12/23:20:38:25.149326666user 9\n"
                    "fc0244c8-ebc6-458a-a8bc-f4fdbe01082f06/12/30:20:38:25.149326666user 10\n";

  std::ofstream stream(path / "session.db");

  stream.clear();
  stream << data;
  stream.close();

  AuthAgent cache(session_cache_size, path, session_ttl);
  uuid_t uuid;
  int i;

  const auto uuids = {"334dc436-d22c-487c-84ce-cf9e369d5a32\0", "1fade928-1b0b-4cbc-8773-1e5a1484f1d2\0",
                      "54894c59-eb8f-4776-ad7e-577271620697\0", "36738adf-332c-40f0-9dbc-c1ae9fc6df8f\0",
                      "08b92265-9f6e-4ab0-bc47-618f5021fc84\0", "fc0244c8-ebc6-458a-a8bc-f4fdbe01082f\0"};

  const auto bad_uuids = {"542f9292-d998-4d5e-bf00-226b68ed677d", "b0a392ab-2fd3-4236-8dee-44d75efca117",
                          "7eb9aba8-4959-4c66-ad4c-9bfa63edfb4f"};

  for (const auto& id : uuids)
  {
    uuid_parse(id, uuid);

    if ((i = cache.fetch(uuid)) == -1)
    {
      err("could not fetch user " + std::string(id));
      return true;
    }
  }

  for (const auto& id : bad_uuids)
  {
    uuid_parse(id, uuid);

    if ((i = cache.fetch(uuid)) != -1)
    {
      err("fetched invalid uuid: " + std::string(id));
      err(cache[i].user);
      return true;
    }
  }

  std::string str, line;
  std::ifstream in(path / "session.db");
  while (std::getline(in, line))
    str += line + "\n";

  if (str != "334dc436-d22c-487c-84ce-cf9e369d5a3206/12/30:20:38:25.149326666user 1\n"
             "1fade928-1b0b-4cbc-8773-1e5a1484f1d206/12/30:20:38:25.149326666user 3\n"
             "54894c59-eb8f-4776-ad7e-57727162069706/12/30:20:38:25.149326666user 6\n"
             "36738adf-332c-40f0-9dbc-c1ae9fc6df8f06/12/30:20:38:25.149326666user 7\n"
             "08b92265-9f6e-4ab0-bc47-618f5021fc8406/12/30:20:38:25.149326666user 8\n"
             "fc0244c8-ebc6-458a-a8bc-f4fdbe01082f06/12/30:20:38:25.149326666user 10\n")
  {
    err(str);

    return true;
  }

  return false;
}

bool test_gen()
{
  auto path = std::filesystem::current_path() / "test" / "data" / "gen";
  std::vector<Session> uuids;
  AuthAgent cache(session_cache_size, path, session_ttl);
  bool res = false;

  for (int i = 0; i < session_cache_size * 10; i++)
    uuids.push_back(cache.generate("name"));

  for (const auto& uuid : uuids)
  {
    if (cache.fetch(uuid.uuid) == -1)
    {
      err("user not found");
      res = true;
      break;
    }
  }

  std::ofstream stream(path / "session.db", std::ios::out | std::ios::trunc);
  stream << "";

  return res;
}

bool test_password()
{
  auto path = std::filesystem::current_path() / "test" / "data" / "pass";
  AuthAgent cache(session_cache_size, path, session_ttl);

  cache.save_password_hash("user", "12345good-@$@-password<3");
  return !cache.invalidate_password("user", "12345good-@$@-password<3") &&
         cache.invalidate_password("user", "fkdl;safjkd;ffk;ajf;dklsa;fjdksfjdsk");
}

int session(int argc, char* argv[])
{
  bool o = 0;

  if (o += test_load()) err("load failed");
  log("load passed");
  if (o += test_gen()) err("gen failed");
  log("gen passed");
  if (o += test_password()) err("password failed");
  log("password passed");

  return o;
}
