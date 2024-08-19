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
  std::string cmd = _message.substr(0, _message.find("\r\n"));

  size_t index = cmd.find(" ");

  switch (index) {
  case 3:
    if (cmd.compare(0, 3, "GET") == 0) {
      message.cmd.method = Method::GET;
      break;
    } else if (cmd.compare(0, 3, "POST") == 0) {
      message.cmd.method = Method::PUT;
      break;
    }
  case 4:
    if (cmd.compare(0, 4, "HEAD") == 0) {
      message.cmd.method = Method::HEAD;
      break;
    } else if (cmd.compare(0, 4, "POST") == 0) {
      message.cmd.method = Method::POST;
      break;
    }
  case 5:
    if (cmd.compare(0, 5, "TRACE") == 0) {
      message.cmd.method = Method::TRACE;
      break;
    }
  case 6:
    if (cmd.compare(0, 6, "DELETE") == 0) {
      message.cmd.method = Method::DELETE;
      break;
    }
  case 7:
    if (cmd.compare(0, 7, "CONNECT") == 0) {
      message.cmd.method = Method::CONNECT;
      break;
    } else if (cmd.compare(0, 7, "OPTIONS") == 0) {
      message.cmd.method = Method::OPTIONS;
      break;
    }
  default:
    exit(1);
  };

  message.cmd.path = cmd.substr(index + 1, cmd.rfind(" ") - index);
  message.cmd.protocol = cmd.substr(cmd.rfind(" ") + 1);

  std::string::size_type contentLength = _message.find("Content-Length: ");

  if (contentLength != std::string::npos) {
    contentLength += 16;
    size_t i = contentLength;

    while (_message[++i] != '\r')
      ;

    message.body = _message.substr(atoi(_message.substr(contentLength, i - contentLength).c_str()));

    size_t ancer = i = cmd.size();
    for(; i < _message.size(); i++)
    {
      

    }
  }

  return message;
};
