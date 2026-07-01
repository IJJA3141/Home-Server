#include "epoll.hpp"
#include "policy.hpp"
#include "reverse_proxy.hpp"
#include <cstdint>
#include <functional>
#include <sys/epoll.h>

struct test
{
  void read() {};
  void write() {};
  void half_closed() {};
  void error() {};
  void closed() {};
  int socket;
};

int main(void)
{
  test t;
  Epoll epoll;
  {
    auto handler = epoll.add<EPOLLIN | EPOLLOUT | EPOLLRDHUP>(t);
  }
  epoll.del(t);
}
