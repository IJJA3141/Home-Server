#include <fstream>
#include <iterator>
#include <sstream>

#include "../log.hpp"
#include "parser.hpp"

Router::Router(std::string _execPath)
{
  this->execPath_ = _execPath.substr(0, _execPath.rfind("/"));
  return;
};

void Router::add(Method _method, std::string _path, std::function<std::string(void)> *_lambda,
                 bool _needSSL)
{
  if (_path[_path.size() - 1] == '/') _path = _path.substr(0, _path.size() - 1);

  Page &ref = this->paths_[_path];

  if (ref.methods[_method] != nullptr) {
    std::string method;

    switch (_method) {
    case GET:
      method = "GET";
      break;
    case HEAD:
      method = "HEAD";
      break;
    case POST:
      method = "POST";
      break;
    case PUT:
      method = "PUT";
      break;
    case DELETE:
      method = "DELETE";
      break;
    case CONNECT:
      method = "CONNECT";
      break;
    case OPTIONS:
      method = "OPTIONS";
      break;
    case TRACE:
      method = "TRACE";
      break;
    }

    WARN(method << " on path " << _path
                << " has already been recorded and will thus be overwritten.");
  }

  ref.methods[_method] = _lambda;
  ref.requireSSL = _needSSL;

  return;
};

std::string Router::respond(std::string _message, Client::Type _clientType) const
{

  std::ifstream steam((this->execPath_ + "/html/index.html"));
  if (!steam.is_open()) VERBERR("file opening failed");

  std::string str((std::istreambuf_iterator<char>(steam)), std::istreambuf_iterator<char>());
  PRINT(str);
  steam.close();

  return str;
};

Message parse(std::string _message)
{
  std::stringstream ss(_message);
  ss >> _message;

  Message message;
  message.failure = Message::Failure::NONE;

  switch (_message.size()) {
  case 3:
    if (_message.compare(0, 3, "GET") == 0) {
      message.cmd.method = Method::GET;
      break;
    } else if (_message.compare(0, 3, "POST") == 0) {
      message.cmd.method = Method::PUT;
      break;
    }
  case 4:
    if (_message.compare(0, 4, "HEAD") == 0) {
      message.cmd.method = Method::HEAD;
      break;
    } else if (_message.compare(0, 4, "POST") == 0) {
      message.cmd.method = Method::POST;
      break;
    }
  case 5:
    if (_message.compare(0, 5, "TRACE") == 0) {
      message.cmd.method = Method::TRACE;
      break;
    }
  case 6:
    if (_message.compare(0, 6, "DELETE") == 0) {
      message.cmd.method = Method::DELETE;
      break;
    }
  case 7:
    if (_message.compare(0, 7, "CONNECT") == 0) {
      message.cmd.method = Method::CONNECT;
      break;
    } else if (_message.compare(0, 7, "OPTIONS") == 0) {
      message.cmd.method = Method::OPTIONS;
      break;
    }
  default:
    PRINT(_message.size());
    ERR("unknown method.");
    exit(1);
  };

  ss >> message.cmd.path;
  ss >> message.cmd.protocol;

  std::string buf;
  ss >> buf;
  ss >> _message;

  message.headers[buf] = _message;

  ss >> buf;

  if(buf == _message) exit(1);

  ss >> _message;

  return message;
};
