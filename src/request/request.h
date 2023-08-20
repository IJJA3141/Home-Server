#ifndef REQUEST
#define REQUEST

#include <cstddef>
#include <string>

namespace http {

class Request {
public:
  enum class State {
    OK,
    ERROR_METHOD,
    ERROR_CONNECTION,
    ERROR_VERSION,
    ERROR_PATH,
    ERROR_ACCEPT,
  };

  enum class Method {
    GET,
    HEAD,
    POST,
    PUT,
    DELETE,
    CONNECT,
    OPTIONS,
    TRACE,
    PATCH, e
  };

  enum class Connection {
    keep_alive,
    close,
  };

  Request(State _state, Method _method, Connection _connection, double _version,
          std::string _path, std::string _accept);

  const State state;
  const Method method;
  const Connection connection;
  const double version;
  const std::string path;
  const std::string accept;
};
} // namespace http

#endif // !REQUEST
