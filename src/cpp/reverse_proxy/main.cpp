#include "epoll.hpp"
#include "policy.hpp"
#include "reverse_proxy.hpp"
#include <cstdint>
#include <functional>
#include <sys/epoll.h>

struct test
{
  void notify_read() {};
  void notify_write() {};
  void notify_half_close() {};
  void notify_error() {};
  void notify_close() {};
  inline int fd() { return socket; }
  int socket;
};

int main(void)
{
  test t;
  Epoll epoll;
  constexpr auto flags = EPOLLIN | EPOLLOUT | EPOLLRDHUP;
  auto handler = epoll.add<flags>(t);

  // do stuff

  epoll.del(t);
}
