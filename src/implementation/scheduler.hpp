#pragma once

#include "../server/server.hpp"

#include <chrono>
#include <thread>
#include <vector>

class Scheduler
{
public:
  bool running;

  Scheduler(std::chrono::minutes _interval, std::vector<Tcp *> _server_array);
  void start();
  void stop();

private:
  std::chrono::minutes interval_;
  std::vector<Tcp*> server_array_;
  std::thread thread_;
};
