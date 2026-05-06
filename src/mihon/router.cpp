#include "router.hpp"
#include "../logger/logger.hpp"

void RouterBuilder::add(Method _method, const std::string& _path, Handler _handler)
{
  auto log = Logger::New();
  auto index = (size_t)_method;

  if (this->segments.contains(_path))
  {
    Handler& handler = this->segments[_path].handlers[index];
    if (handler)
    {
      log.crit("{} {} already registered", protocol::HTTP::method_to_string(_method), _path);
      throw "";
    }

    handler = _handler;
  }

  else this->segments[_path].handlers[index] = _handler;
}

void RouterBuilder::add(Method _method, const std::string& _path, HandlerStatic _handler)
{
  auto log = Logger::New();
  auto index = (size_t)_method;

  if (this->segments_static.contains(_path))
  {
    HandlerStatic& handler = this->segments_static[_path].handlers[index];
    if (handler)
    {
      log.crit("{} {} already registered", protocol::HTTP::method_to_string(_method), _path);
      throw "";
    }

    handler = _handler;
  }

  else this->segments_static[_path].handlers[index] = _handler;
}

Router RouterBuilder::build()
{

}
