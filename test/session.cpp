#include "../src/io/auth.hpp"
#include "../src/log.hpp"

#include <filesystem>
#include <fstream>
#include <string>
#include <uuid/uuid.h>
#include <vector>

bool test_load()
{
  auto path = std::filesystem::current_path().append("test").append("data").append("load.db");

  session_cache cache(path);
  uuid_t uuid;

  auto uuids = {"334dc436-d22c-487c-84ce-cf9e369d5a32\0", "1fade928-1b0b-4cbc-8773-1e5a1484f1d2\0",
                "77d3fe85-a100-46d7-bf28-112a0b74e395\0", "542f9292-d998-4d5e-bf00-226b68ed677d\0",
                "b0a392ab-2fd3-4236-8dee-44d75efca117\0", "54894c59-eb8f-4776-ad7e-577271620697\0",
                "36738adf-332c-40f0-9dbc-c1ae9fc6df8f\0", "08b92265-9f6e-4ab0-bc47-618f5021fc84\0",
                "7eb9aba8-4959-4c66-ad4c-9bfa63edfb4f\0", "fc0244c8-ebc6-458a-a8bc-f4fdbe01082f\0"};

  for (auto id : uuids)
  {
    uuid_parse(id, uuid);

    if (cache.fetch(uuid).user.empty())
    {
      err("could not fetch user " + std::string(id));
      return true;
    }
  }

  return false;
}

bool test_gen()
{
  auto path = std::filesystem::current_path().append("test").append("data").append("gen.db");
  std::vector<session_id> uuids;
  session_cache cache(path);
  bool res = false;

  for (int i = 0; i < 10; i++)
    uuids.push_back(cache.generate("name"));

  for (const auto& uuid : uuids)
  {
    if (cache.fetch(uuid.uuid).user.empty())
    {
      err("user not found");
      res = true;
      break;
    }
  }

  std::ofstream stream(path);
  stream.clear();

  return res;
}

int session(int argc, char* argv[]) { 
  bool o = 0;

  if(o += test_load()) err("load failed");
  //if(o += test_gen()) err("gen failed");
  
  return o;
}
