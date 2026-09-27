#pragma once

#include "../protocol/ipcp/ipcp.hpp"

namespace ipc
{

struct IClient
{
  virtual void transmit(protocol::ipcp::Request request) = 0;
};

} // namespace ipc
