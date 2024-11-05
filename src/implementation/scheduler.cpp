#include "scheduler.hpp"
#include <chrono>
#include <thread>

void Scheduler::run()
{
  while (this->running) {
    for (int i = 0; i < 6 * 24 * 7; i++) {
      // action 1
      // action 2
      // ...

      std::this_thread::sleep_for(std::chrono::minutes(10));
    }

    // action 3
    // action 4
    // ...
    
    std::this_thread::sleep_for(std::chrono::minutes(10));
  }
}
