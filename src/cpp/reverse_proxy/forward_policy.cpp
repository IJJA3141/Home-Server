#include "../common/iterator.hpp"
#include "../protocol/http/http.hpp"
#include "policy.hpp"
#include <cstdio>
#include <optional>
#include <sstream>
#include <string>

size_t ForwardPolicy::parse(std::span<const std::byte> stream)
{
  Iterator iterator{stream};

  switch (this->state)
  {
  case State::METHOD:
    if (!iterator.next(' '))
    {
      this->state = State::METHOD;
      break;
    }
    else
    {
      auto method = http::parse_method(iterator.head);
      if (method) this->request.method = method.value();
      else
      {
        this->state = State::FAILED;
        break;
      }
    }

  case State::PATH:
    if (!iterator.next(' '))
    {
      this->state = State::PATH;
      break;
    }
    this->request.path = iterator.head;

  case State::VERSION:
    if (!iterator.next("\r\n"))
    {
      this->state = State::VERSION;
      break;
    }
    else
    {
      auto version = http::parse_version(iterator.head);
      if (version) this->request.version = version.value();
      else
      {
        this->state = State::FAILED;
        break;
      }
    }

  case State::HEADERS:
    if (!iterator.next("\r\n"))
    {
      this->state = State::HEADERS;
      break;
    }

    while (!iterator.head.empty())
    {
      auto header = http::parse_header(iterator.head);
      if (header)
      {
        if (header->first == "content-length")
        {
          int i;
          if (std::sscanf(header->second.c_str(), "%i", &i) != 1 || i < 0)
          {
            this->state = State::FAILED;
            break;
          }
        }
        else if (header->first == "host") this->request.host = header->second;
        else if (header->first == "user-agent") this->request.user_agent = header->second;
        // add more ?
        else this->request.header.emplace(header.value());
      }
      else
      {
        this->state = State::FAILED;
        break;
      }

      if (!iterator.next("\r\n"))
      {
        this->state = State::HEADERS;
        break;
      }
    }

  case State::FORWARDING:
    if (this->request.content_length == 0)
    {
      this->state = State::DONE;
    }
    else if (this->received + iterator.tail.size() >= this->request.content_length)
    {
      this->state = State::DONE;
      stream.subspan(stream.size() - iterator.tail().size(), this->request.content_length - this->received);
    }
    else
    {
    }

    break;

  case State::DONE:
  case State::FAILED:
  }

  return stream.size() - iterator.tail.size();
}
