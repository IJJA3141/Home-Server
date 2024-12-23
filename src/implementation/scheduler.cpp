#include "scheduler.hpp"
#include <chrono>
#include <thread>

Scheduler::Scheduler(std::chrono::minutes _interval, std::vector<Tcp *> _server_array)
{
  this->interval_ = _interval;
  this->server_array_ = _server_array;

  return;
};

void Scheduler::start()
{
  this->running = true;
  this->thread_ = std::thread([this](void(void)) -> void {
    while (this->running) {
      std::this_thread::sleep_for(this->interval_);

      for (int i = 0; i < this->server_array_.size(); i++) {
        this->server_array_[i]->update_client_state();
        this->server_array_[i]->clean_client_array();
      }
    };

    return;
  });

  return;
}

void Scheduler::stop()
{
  this->running = false;
  this->thread_.join();

  return;
}
