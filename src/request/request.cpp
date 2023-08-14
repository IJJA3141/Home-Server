#include "./request.h"

http::Request::Request(State _state, Method _method, Connection _connection,
                       double _version, std::string _path, std::string _accept)
    : state(_state), method(_method), connection(_connection),
      version(_version), path(_path), accept(_accept) {
  return;
}
