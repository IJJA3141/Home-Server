#include "policy.hpp"
#include "reverse_proxy.hpp"

int main(void)
{
  TcpServer server("", 0, [](ForwardPolicy::Request&, ForwardPolicy::Response&) -> void {});
}
