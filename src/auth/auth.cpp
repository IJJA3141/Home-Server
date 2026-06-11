#include "auth.hpp"

// using HTTP = protocol::HTTP;
// using ATP = protocol::ATP;
//
// protocol::Middleware<HTTP> create_auth_middleware(const std::string& login, const std::string& source,
//                                                   ipc::TransportClient<ATP>& ipc_client)
// {
//   return [&](HTTP::Request& _request, protocol::Handler<HTTP> _callback) -> HTTP::Response {
//     if (_request.headers[HTTP_CONNECTION_TYPE] != CONNECTION_TYPE_HTTPS)
//       return {HTTP::Version::HTTP_11, 308, "Permanent Redirect", {{"location", "https://" + _request.path}}};
//
//     std::map<std::string, std::string> cookies = HTTP::parse_cookies(_request.headers["cookie"]);
//     std::optional<Uuid> uuid;
//
//     if (cookies.contains(HTTP_SESSION_ID) && (uuid = Uuid::parse_safe(cookies[HTTP_SESSION_ID])))
//     {
//       auto res = ipc_client.transmit(
//           {ATP::Type::VAL, ATP::VAL::Request{source, uuid.value(), _request.headers[HTTP_CLIENT_IP],
//                                              _request.headers["user-agent"]}});
//
//       if (res.type != ATP::Type::VAL) throw;
//       auto r = std::get<ATP::VAL::Response>(res.response);
//
//       if (r.uuid)
//       {
//         _request.headers[HTTP_UUID] = r.uuid.value();
//         return _callback(_request);
//       }
//     }
//
//     return {
//         protocol::HTTP::Version::HTTP_11, 303, "See Other", {{"location", login + "?returnto=" + _request.path}}};
//   };
// }
