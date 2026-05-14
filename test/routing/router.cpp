#include "../../src/routing/routing.hpp"
#include "../../src/utils/exception.hpp"
#include "../test.hpp"
#include <array>
#include <string_view>

using namespace test;

using Method = protocol::HTTP::Method;
using Request = protocol::HTTP::Request;
using Response = protocol::HTTP::Response;
using Handler = protocol::Handler<protocol::HTTP>;
using Middleware = protocol::Middleware<protocol::HTTP>;

Response dummy(Request&) { return {}; }

constexpr Handler assert_handler(uint& flag, uint i)
{
  return [&, i](auto&) -> Response {
    flag |= i;
    return {};
  };
}

constexpr Middleware assert_middleware(uint& flag, uint i)
{
  return [&, i](auto& req, auto fn) -> Response {
    flag |= i;
    return fn(req);
  };
}

template <size_t N> consteval std::array<uint, N> get_flags()
{
  std::array<uint, N> arr;

  for (size_t i = 0; i < N; ++i)
    arr[i] = 1 << i;

  return arr;
}

int routing_router(int argc, char* argv[])
{
  int _flag = 0, _i = 0, _j = 0;
  bool _exception_caught = false;

  SECTION("router - static routes")
  {
    Router r;
    REQUIRE("register root for every method", {
      for (size_t i = 0; i < N_METHODS; ++i)
        r.add((Method)i, "/", dummy);
    });

    // fails DAP
    for (size_t i = 0; i < N_METHODS; ++i)
    {
      REQUIRE_THROW(std::format("duplicate method {} conflict", protocol::HTTP::method_to_string((Method)i)),
                    configuration_error, { r.add((Method)i, "/", dummy); });
    }
  }

  SECTION("router - parametric routes")
  {
    Router r;
    REQUIRE("simple param", { r.add(Method::GET, "/root/parametric/[user]/test", dummy); });
    REQUIRE("same param prefix different leaf", { r.add(Method::GET, "/root/parametric/[user]/other", dummy); });
    REQUIRE("param + wildcard", { r.add(Method::GET, "/parametric/[test]//*", dummy); });
    REQUIRE("param ending with slash", { r.add(Method::GET, "/parametric/[test]/", dummy); });
    REQUIRE_THROW("static conflicts with param branch", configuration_error,
                  { r.add(Method::GET, "/root/parametric/other", dummy); });
  }

  SECTION("router - complex param trees")
  {
    Router r;
    REQUIRE("param at root", { r.add(Method::GET, "/[parametric]/test/", dummy); });
    REQUIRE("multiple params deep", { r.add(Method::GET, "/[parametric]/other/[a]/[b]/leaf", dummy); });
  }

  SECTION("router - wildcard")
  {
    Router r;
    REQUIRE("simple wildcard", { r.add(Method::GET, "/static/*", dummy); });
    REQUIRE_THROW("duplicate wildcard route", configuration_error, { r.add(Method::GET, "/static/*", dummy); });
    REQUIRE_THROW("after wildcard route", configuration_error, { r.add(Method::GET, "/static/test/", dummy); });
  }

  ENDSECTION;

  _flag = 0, _i = 0, _j = 0;
  _exception_caught = false;

  SECTION("router - resolution static")
  {
    REQUIRE("handler invocation", {
      Router r;

      bool flag = false;
      r.add(Method::GET, "/", [&](auto&) -> Response {
        flag = true;
        return {};
      });

      Request req(Method::GET, "/");

      r.handle(req);
      assert(flag);
    });

    Router r;
    uint flag = 0;
    std::array reqs{Request(Method::GET, "/"), Request(Method::PUT, "/"), Request(Method::GET, "/a"),
                    Request(Method::GET, "/a/b"), Request(Method::GET, "/a/b?test=abc")};
    std::array flags{get_flags<5>()};

    r.add(reqs[0].method, reqs[0].path, assert_handler(flag, flags[0]));
    r.add(reqs[1].method, reqs[1].path, assert_handler(flag, flags[1]));
    r.add(reqs[2].method, reqs[2].path, assert_handler(flag, flags[2]));
    r.add(reqs[3].method, reqs[3].path, assert_middleware(flag, flags[3]), assert_handler(flag, flags[4]));

    REQUIRE("static handlers resolve by method", {
      r.handle(reqs[0]);
      assert_equal(flag, flags[0]);

      r.handle(reqs[1]);
      assert_equal(flag, flags[0] | flags[1]);

      r.handle(reqs[2]);
      assert_equal(flag, flags[0] | flags[1] | flags[2]);
    });

    REQUIRE("static handlers & middleware resolve by method", {
      flag = 0;
      r.handle(reqs[3]);
      assert_equal(flag, flags[3] | flags[4]);
    });

    REQUIRE("handle query in request", {
      flag = 0;
      r.handle(reqs[4]);
      assert_equal(flag, flags[3] | flags[4]);
    });
  }

  SECTION("router - resolution parametric")
  {
    Router r;
    uint flag = 0;

    std::array reqs{
        Request(Method::GET, "/user/42/pp"),
        Request(Method::PUT, "/user/42/pp"),
        Request(Method::GET, "/post/10/comment/5"),
        Request(Method::GET, "/post/99/comment/77"),
    };

    std::array flags{get_flags<4>()};

    r.add(Method::GET, "/user/[id]/pp", assert_handler(flag, flags[0]));
    r.add(Method::PUT, "/user/[id]/pp", assert_handler(flag, flags[1]));
    r.add(Method::GET, "/post/[post]/comment/[comment]/", assert_middleware(flag, flags[2]),
          assert_handler(flag, flags[3]));

    REQUIRE("parametric handlers resolve by method", {
      r.handle(reqs[0]);
      assert_equal(flag, flags[0]);

      r.handle(reqs[1]);
      assert_equal(flag, flags[0] | flags[1]);
    });

    REQUIRE("multi parametric path resolves", {
      flag = 0;

      r.handle(reqs[2]);
      assert_equal(flag, flags[2] | flags[3]);
    });

    REQUIRE("parametric segments accept different values", {
      flag = 0;

      r.handle(reqs[3]);
      assert_equal(flag, flags[2] | flags[3]);
    });

    REQUIRE("parametric segments match right vlaue", {
      std::string id;
      std::string time;
      r.add(
          Method::GET, "/package/[id]/a/[time]/leaf",
          [&](auto& req, auto fn) -> Response {
            id = req.headers["x-id"];
            return fn(req);
          },
          [&](auto& req) -> Response {
            time = req.headers["x-time"];
            return {};
          });

      Request r1(Method::GET, "/package/23/a/10h/leaf");
      Request r2(Method::GET, "/package/28/a/1h/leaf");

      r.handle(r1);
      assert_equal<std::string_view>(id, "23");
      assert_equal<std::string_view>(time, "10h");

      r.handle(r2);
      assert_equal<std::string_view>(id, "28");
      assert_equal<std::string_view>(time, "1h");
    });
  }

  SECTION("router - resolution wildcard")
  {
    Router r;
    uint flag = 0;

    std::array reqs{
        Request(Method::GET, "/static/js/app.js"), Request(Method::PUT, "/static/css/style.css"),
        Request(Method::GET, "/files/a/b/c.txt"),  Request(Method::GET, "/files/x/y/z.log"),
        Request(Method::GET, "/files/"),
    };

    std::array flags{get_flags<4>()};

    r.add(Method::GET, "/static/*", assert_handler(flag, flags[0]));
    r.add(Method::PUT, "/static/*", assert_handler(flag, flags[1]));
    r.add(Method::GET, "/files/*", assert_middleware(flag, flags[2]), assert_handler(flag, flags[3]));

    REQUIRE("wildcard resolves per method", {
      r.handle(reqs[0]);
      assert_equal(flag, flags[0]);
      assert_equal<std::string_view>(reqs[0].headers["x-wildcard"], "js/app.js");

      r.handle(reqs[1]);
      assert_equal(flag, flags[0] | flags[1]);
      assert_equal<std::string_view>(reqs[1].headers["x-wildcard"], "css/style.css");
    });

    REQUIRE("wildcard captures deep paths", {
      flag = 0;

      r.handle(reqs[2]);
      assert_equal(flag, flags[2] | flags[3]);
      assert_equal<std::string_view>(reqs[2].headers["x-wildcard"], "a/b/c.txt");
    });

    REQUIRE("wildcard matches arbitrary suffix", {
      flag = 0;

      r.handle(reqs[3]);
      assert_equal(flag, flags[2] | flags[3]);
      assert_equal<std::string_view>(reqs[3].headers["x-wildcard"], "x/y/z.log");
    });

    REQUIRE("wildcard matches empty suffix", {
      r.handle(reqs[4]);
      assert_equal<std::string_view>(reqs[4].headers["x-wildcard"], "");
    });
  }

  SECTION("router - parametric x wildcard")
  {
    Router r;
    Request req(Method::PUT, "/user/67/static/super_thisgs/");

    std::string id;
    std::string tail;
    r.add(Method::PUT, "/user/[id]/static/*", [&](auto& req) -> Response {
      id = req.headers["x-id"];
      tail = req.headers["x-wildcard"];
      return {};
    });

    r.handle(req);
    assert_equal<std::string_view>(id, "67");
    assert_equal<std::string_view>(tail, "super_thisgs");
  }

  ENDSECTION;

  return _flag;
}
