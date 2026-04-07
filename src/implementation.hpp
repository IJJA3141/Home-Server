#pragma once

#include "network/http.hpp"

#include <filesystem>
#include <string>

std::string load(std::filesystem::path _path);

struct Response : public http::Response
{
  Response(const int _status, const std::string _body, const http::Protocol _protocol = http::Protocol::HTTP_11);
};

namespace handler
{
extern http::Response internal_error;
extern http::Response not_implemented;
extern http::Response moved;
extern http::Response not_found;
extern http::Response method_not_allowed;
extern http::Response unauthorized;
extern http::Response login_failed;
} // namespace handler
