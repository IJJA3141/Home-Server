#pragma once

#include "../protocol/http/http.hpp"
#include "../protocol/http/iterator.hpp"
#include <memory>
#include <vector>

// using Handler = protocol::Handler<protocol::HTTP>;
// using HandlerStatic = std::function<protocol::HTTP::Response(protocol::HTTP::Request&, std::string_view)>;
// using HandlerAuthenticated = std::function<protocol::HTTP::Response(protocol::HTTP::Request&,
// std::string_view)>;
//
// using Method = protocol::HTTP::Method;
//
// struct ISegment;
// using table_value_t = std::pair<std::string, std::unique_ptr<ISegment>>;
// using table_t = std::vector<table_value_t>;
//
// inline size_t find(const table_t& table, std::string_view key)
// {
//   for (size_t i = 0; i < table.size(); ++i)
//     if (table[i].first == key) return i;
//
//   return -1;
// }

struct ISegment
{
  using Request = protocol::HTTP::Request;
  using Response = protocol::HTTP::Response;

  using Value = std::pair<std::string, std::unique_ptr<ISegment>>;
  using Table = std::vector<Value>;

  virtual ~ISegment();

  virtual Response resolve(Iterator& path, Request& request) = 0;
  virtual void insert(Iterator& path, std::unique_ptr<ISegment>& segment) = 0;

  virtual Table get_table() = 0;
  virtual void set_table(Table table) = 0;
};

struct EmptySegment : ISegment
{
  EmptySegment(Iterator& path, std::unique_ptr<ISegment>& segment);

  Response resolve(Iterator& path, Request& request) override;
  void insert(Iterator& path, std::unique_ptr<ISegment>& segment) override;

  Table get_table() override;
  void set_table(Table table) override;

private:
  Table table_;
};

// struct Segment : ISegment
// {
//   Handler handlers[N_METHODS];
//   table_t table;
//
//   /**
//    * @brief
//    *
//    * @param  path  Remaining path to @a _segment. Tailing '/' needs not be removed. Can be empty.
//    * @param  _segment
//    *
//    * @return
//    */
//   void insert(Iterator& path, std::unique_ptr<ISegment>& _segment) override
//   {
//     assert(path.tail.empty() || path.tail.back() != '/');
//     size_t i = find(this->table, path.head);
//
//     if (path.next('/'))
//     { // not the end segment
//       if (i < this->table.size())
//       { // next segment is already registered reple down
//         this->table[i].second->insert(path, _segment);
//       }
//       else
//       { // no next segment add an empty one for now
//         this->table.emplace_back(path.head, std::make_unique<EmptySegment>(path, _segment));
//       }
//     }
//     else
//     { // path is end segment
//       if (i < this->table.size())
//       { // segment is already registered
//         _segment->adopt_children(this->table[i].second->steal_children());
//         std::swap(this->table[i].second, _segment);
//       }
//       else
//       { // add new segment
//         this->table.emplace_back(path.head, std::move(_segment));
//       }
//     }
//   }
//
//   void adopt_children(table_t&& _table) override { std::swap(this->table, _table); }
// };
//
// struct AuthSegment : Segment
// {
// };
//
// struct StaticSegment : ISegment
// {
//   HandlerStatic handlers[N_METHODS];
//
//   void insert(Iterator& path, std::unique_ptr<ISegment>& segment) override { throw ""; }
//   void adopt_children(table_t&&) override { throw ""; }
// };
//
// struct ParamSegment : ISegment
// {
// };
//
// class Router
// {
// };
//
// class RouterBuilder
// {
//
// public:
//   void add(Method method, const std::string& path, Handler handler);
//   void add(Method method, const std::string& path, HandlerStatic handler);
//   // void add(Method method, std::string_view path, HandlerAuthenticated handler);
//
//   Router build();
//
// private:
//   std::map<std::string, StaticSegment> segments_static;
//   std::map<std::string, AuthSegment> segments_auth;
//   std::map<std::string, Segment> segments;
// };
