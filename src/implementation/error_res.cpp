#include "../implementation.hpp"
#include "../network/http.hpp"

Response::Response(const int _status, const std::string _body, const http::Protocol _protocol)
    : http::Response(_protocol, _status, {{"Strict-Transport-Security", "max-age=31536000"}}, _body) {};

http::Response handler::internal_error{Response(HTTP_INTERNAL_SERVER_ERROR, "fuck !")};
http::Response handler::not_implemented{Response(HTTP_NOT_IMPLEMENTED, "")};
http::Response handler::moved{Response(HTTP_MOVED_PERMANENTLY, "")};
http::Response handler::not_found{Response(HTTP_NOT_FOUND, "")};
http::Response handler::method_not_allowed{Response(HTTP_METHOD_NOT_ALLOWED, "")};
http::Response handler::unauthorized{Response(HTTP_UNAUTHORIZED, "")};
http::Response handler::login_failed{Response(HTTP_UNAUTHORIZED, "login failed")};
