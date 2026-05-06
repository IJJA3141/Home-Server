#include "router.hpp"
#include "../../protocol/http/http.cpp"

using Request = protocol::HTTP::Request;
using Response = protocol::HTTP::Response;

struct ISegment;
using Table = std::unordered_map<std::string, std::unique_ptr<ISegment>>;

Response EmptySegment::resolve(Iterator& path, Request& request) override;
void insert(Iterator& path, std::unique_ptr<ISegment>& segment) override;

Table get_table() override;
void set_table(Table table) override;
