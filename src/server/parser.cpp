#include <fstream>
#include <iterator>

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
  Message message;
  message.failure = Message::Failure::NONE;

  switch (_message[0]) {
  case 'G':
    if (_message.compare(0, 3, "GET") == 0) {
      message.cmd.method = Method::GET;
      break;
    }
  case 'H':
    if (_message.compare(0, 4, "HEAD") == 0) {
      message.cmd.method = Method::HEAD;
      break;
    }
  case 'P':
    if (_message[1] == 'O') {
      if (_message.compare(0, 4, "POST") == 0) {
        message.cmd.method = Method::POST;
        break;
      }
    } else if (_message[1] == 'U') {
      if (_message.compare(0, 4, "POST") == 0) {
        message.cmd.method = Method::POST;
        break;
      }
    }
  case 'D':
    if (_message.compare(0, 6, "DELETE") == 0) {
      message.cmd.method = Method::DELETE;
      break;
    }
  case 'C':
    if (_message.compare(0, 7, "CONNECT") == 0) {
      message.cmd.method = Method::CONNECT;
      break;
    }
  case 'O':
    if (_message.compare(0, 7, "OPTIONS") == 0) {
      message.cmd.method = Method::OPTIONS;
      break;
    }
  case 'T':
    if (_message.compare(0, 5, "TRACE") == 0) {
      message.cmd.method = Method::TRACE;
      break;
    }
  default:
    ERR("");
    exit(1);
  };


};
