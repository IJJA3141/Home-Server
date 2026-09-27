#include "tls.hpp"
#include <expected>

using namespace asio;
using namespace tls;

Awaitable<std::expected<Socket, Error>> Acceptor::async_accept()
{
  return {this->fd_, this->executor_, Executor::Operation::read, std::unexpected(Error::none),
          [this](auto& value) {
            std::expected<tcp::Socket, Error> val{std::unexpected(Error::none)};

            Executor::Operation op = tcp::Acceptor::accept(val);
            if (op == Executor::Operation::end)
            { // move val to value
              if (val) value = Socket(std::move(val.value()), this->ctx_);
              else value = std::unexpected(val.error());
            }

            return op;
          }};
}
