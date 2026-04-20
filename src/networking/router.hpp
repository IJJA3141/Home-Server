#pragma once

#include "http.hpp"

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

class IRoute
{
public:
  IRoute(const std::string& _path);

  virtual bool match(std::string_view _path) const = 0;
  virtual http::Response invoke(http::Request _request) const = 0;

  bool operator==(const IRoute& _other) const { return _other.match(this->path_) || match(_other.path_); };

protected:
  const std::string path_;
};

class Route : public IRoute
{
public:
  Route(const std::string& _path, const std::function<http::Response(http::Request)> _function);

  bool match(const std::string_view _path) const override { return this->path_ == _path; };
  http::Response invoke(const http::Request _request) const override { return this->function_(_request); };

private:
  const std::function<http::Response(http::Request)> function_;
};

class SRoute : public IRoute
{
public:
  SRoute(const std::string& _path, const std::function<http::Response(http::Request, std::string)> _function);

  bool match(std::string_view _path) const override;
  http::Response invoke(const http::Request _request) const override;

private:
  const std::function<http::Response(http::Request, std::string)> function_;
};

class CRoute : public IRoute
{
public:
  CRoute(const std::string& _path, http::Response _response);

  bool match(std::string_view _path) const override { return this->path_ == _path; };
  http::Response invoke(const http::Request _request) const override { return this->response_; };

private:
  const http::Response response_;
};

class Router
{
public:
  Router(const http::Response _fallback);

  void add(const http::Method _method, const std::string_view _path,
           const std::function<http::Response(http::Request)> _function);

  void add(const http::Method _method, const std::string_view _path,
           const std::function<http::Response(http::Request, std::string)> _function);

  void add(const http::Method _method, const std::string_view _path, const http::Response _response);

  void add(const http::Method _method, const std::shared_ptr<IRoute> _route);

  void add(const http::Error _error, const http::Response _response);

  http::Response respond(const http::Request _request) const;
  http::Response handle_error(const http::Error _error) const;

private:
  std::optional<http::Response> error_handlers_[http::error_size];
  std::vector<std::shared_ptr<IRoute>> routes_[http::method_size];
};
