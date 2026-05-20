#pragma once

#include "../../utils/uuid.hpp"
#include "../protocol.hpp"
#include <expected>
#include <netinet/in.h>
#include <optional>
#include <string>
#include <uuid/uuid.h>
#include <variant>

namespace protocol
{

struct ATP
{
  struct VAL
  {
    /**
     * format [source]\n[id]\n[ip]\n[user agent]\n
     */
    struct Request
    {
      std::string source;
      Uuid id;
      std::string ip;
      std::string user_agent;

      class ParserContext;
      static size_t parse(std::span<const char>, ParserContext&);
      operator std::string() const;
    };

    /**
     * format either unparsed uuid or empty string
     */
    struct Response
    {
      std::expected<Uuid, std::string> uuid;

      class ParserContext;
      static size_t parse(std::span<const char>, ParserContext&);
      operator std::string() const;
    };
  };

  struct LOG
  {
    /**
     * format [user]\n[password]\n[ip]\n[user agent]\n
     */
    struct Request
    {
      std::string user;
      std::string pwd;
      std::string ip;
      std::string user_agent;

      class ParserContext;
      static size_t parse(std::span<const char>, ParserContext&);
      operator std::string() const;
    };

    struct Response
    {
      std::expected<Uuid, std::string> id;

      class ParserContext;
      static size_t parse(std::span<const char>, ParserContext&);
      operator std::string() const;
    };
  };

  struct GEN
  {
    /**
     * format [source]\n[id]\n[ip]\n[user agent]\n
     */
    struct Request
    {
      std::string source;
      Uuid id;
      std::string ip;
      std::string user_agent;

      class ParserContext;
      static size_t parse(std::span<const char>, ParserContext&);
      operator std::string() const;
    };

    /**
     * format either unparsed uuid or empty string
     */
    struct Response
    {
      std::expected<Uuid, std::string> uuid;

      class ParserContext;
      static size_t parse(std::span<const char>, ParserContext&);
      operator std::string() const;
    };
  };

  enum struct Type
  {
    // authentification
    VAL, // validate    id, source, ctx -> uuid/failure
    LOG, // login       user, clear b64 pwd, ctx -> id/failure
    GEN, // regenerate  id, source, ctx -> id/failure

    // administration
    // DIR, // list    -> list uuid
    // GET, // get     uuid -> info/failure
    // MOD, // modify  uuid, info -> success/failure
    // ADD, // add     source, user, clear b64 pwd -> success/failure
    // DEL, // delete  uuid -> success/failure
  };

  struct Request
  {
    std::variant<VAL::Request, LOG::Request, GEN::Request> request;

    class ParserContext;
    static size_t parse(std::span<const char>, ParserContext&);
    operator std::string() const
    {
      return request.visit([](auto&& _) -> std::string { return _; });
    };
  };

  struct Response
  {
    std::variant<VAL::Response, LOG::Response, GEN::Response> response;

    class ParserContext;
    static size_t parse(std::span<const char>, ParserContext&);
    operator std::string() const
    {
      return response.visit([](auto&& _) -> std::string { return _; });
    };
  };
};

class ATP::VAL::Request::ParserContext
{
public:
  std::string error_msg;
  ParserResult result = ParserResult::NeedMoreData;
  Request construct();

private:
  enum
  {
    TYPE,
    SOURCE,
    ID,
    IP,
    UA,
  } state_ = SOURCE;
  std::string source_;
  std::optional<Uuid> id_;
  std::string ip_;
  std::string user_agent_;
  friend struct ATP::VAL::Request;
};

class ATP::VAL::Response::ParserContext
{
public:
  std::string error_msg;
  ParserResult result = ParserResult::NeedMoreData;
  Response construct();

private:
  bool typed_;
  std::expected<Uuid, std::string> id_;
  friend struct ATP::VAL::Response;
};

class ATP::LOG::Request::ParserContext
{
public:
  std::string error_msg;
  ParserResult result = ParserResult::NeedMoreData;
  Request construct();

private:
  enum
  {
    TYPE,
    USER,
    PWD,
    IP,
    UA,
  } state_ = USER;
  Request request_;
  friend struct ATP::LOG::Request;
};

class ATP::LOG::Response::ParserContext
{
public:
  std::string error_msg;
  ParserResult result = ParserResult::NeedMoreData;
  Response construct();

private:
  bool typed_;
  std::expected<Uuid, std::string> id_;
  friend struct ATP::LOG::Response;
};

class ATP::GEN::Request::ParserContext
{
public:
  std::string error_msg;
  ParserResult result = ParserResult::NeedMoreData;
  Request construct();

private:
  enum
  {
    TYPE,
    SOURCE,
    ID,
    IP,
    UA,
  } state_ = SOURCE;
  std::string source_;
  std::optional<Uuid> id_;
  std::string ip_;
  std::string user_agent_;
  friend struct ATP::GEN::Request;
};

class ATP::GEN::Response::ParserContext
{
public:
  std::string error_msg;
  ParserResult result = ParserResult::NeedMoreData;
  Response construct();

private:
  bool typed_;
  std::expected<Uuid, std::string> id_;
  friend struct ATP::GEN::Response;
};

class ATP::Request::ParserContext
{
public:
  std::string error_msg;
  ParserResult result = ParserResult::NeedMoreData;
  Request construct();

private:
  std::optional<Type> type_ = std::nullopt;
  VAL::Request::ParserContext val_ctx_;
  LOG::Request::ParserContext log_ctx_;
  GEN::Request::ParserContext gen_ctx_;

  friend struct ATP::Request;
};

class ATP::Response::ParserContext
{
public:
  std::string error_msg;
  ParserResult result = ParserResult::NeedMoreData;
  Response construct();

private:
  std::optional<Type> type_ = std::nullopt;
  VAL::Response::ParserContext val_ctx_;
  LOG::Response::ParserContext log_ctx_;
  GEN::Response::ParserContext gen_ctx_;

  friend struct ATP::Response;
};

constexpr std::optional<ATP::Type> parse_type(std::string_view _type)
{
  if (_type == "VAL") return ATP::Type::VAL;
  else if (_type == "LOG") return ATP::Type::LOG;
  else if (_type == "GEN") return ATP::Type::GEN;
  else return std::nullopt;
}

} // namespace protocol
