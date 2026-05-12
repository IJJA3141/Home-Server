#pragma once

#include "../protocol/http/http.hpp"
#include <memory>
#include <meta>
#include <string>
#include <string_view>
#include <vector>

struct Segment
{
  using Middleware = protocol::Middleware<protocol::HTTP>;
  using Handler = protocol::Handler<protocol::HTTP>;
  using Route = std::pair<std::string, std::unique_ptr<Segment>>;
  using Handlers = std::pair<Middleware, Handler>;
  /**
   * Describes the relationship between this segment and its children.
   * All children of a segment must share the same relationship type.
   * Mixing different types among sibling segments is not allowed.
   */
  enum struct ParentingType
  {
    CHILDLESS,
    STATIC,
    PARAMETRIC,
    WILDCARD
  };

  ParentingType type;
  std::array<Handlers, N_METHODS> handlers;
  std::vector<Route> table;
  std::string meta;

  Segment* find(std::string_view route);
  Segment* get_or_create(std::string_view route);

  void sprint(std::string parent, std::string& out) const;
};

class Router
{
  using Method = protocol::HTTP::Method;
  using Middleware = protocol::Middleware<protocol::HTTP>;
  using Handler = protocol::Handler<protocol::HTTP>;

public:
  Router() : root{std::make_unique<Segment>()} {}

  // void add(std::string_view prefix, Router&& subtree);
  void add(Method method, std::string_view path, Handler handler);
  void add(Method method, std::string_view path, Middleware middleware, Handler handler);

  inline operator std::string() const
  {
    std::string str;
    this->root->sprint("", str);
    return str;
  }

private:
  std::unique_ptr<Segment> root;

  Segment* get(std::string_view path);
};

void assert_valid(const std::string_view path);
std::string_view regularise_path(std::string_view path);
