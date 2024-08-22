#include "parser.hpp"
#include "../log.hpp"

Request parse(const std::string _message)
{
  Stream ss(_message);
  std::string iterator;
  ss >> iterator;

  Request message;
  message.failure = Request::Failure::NONE;

  switch (iterator.size()) {
  case 3:
    if (iterator.compare(0, 3, "GET") == 0) {
      message.cmd.method = Method::GET;
      break;
    } else if (iterator.compare(0, 3, "POST") == 0) {
      message.cmd.method = Method::PUT;
      break;
    }
  case 4:
    if (iterator.compare(0, 4, "HEAD") == 0) {
      message.cmd.method = Method::HEAD;
      break;
    } else if (iterator.compare(0, 4, "POST") == 0) {
      message.cmd.method = Method::POST;
      break;
    }
  case 5:
    if (iterator.compare(0, 5, "TRACE") == 0) {
      message.cmd.method = Method::TRACE;
      break;
    }
  case 6:
    if (iterator.compare(0, 6, "DELETE") == 0) {
      message.cmd.method = Method::DELETE;
      break;
    }
  case 7:
    if (iterator.compare(0, 7, "CONNECT") == 0) {
      message.cmd.method = Method::CONNECT;
      break;
    } else if (iterator.compare(0, 7, "OPTIONS") == 0) {
      message.cmd.method = Method::OPTIONS;
      break;
    }
  default:
    PRINT(iterator.size());
    ERR("unknown method.");
    message.failure = Request::Failure::HEADER;
    return message;
  };

  ss >> iterator;

  if (iterator.size() == 0) {
    ERR("[empty] is not a valid path");
    message.failure = Request::Failure::PATH;
    return message;
  }

  if (iterator[0] != '/') {
    ERR("a path should start with /");
    message.failure = Request::Failure::PATH;
    return message;
  }

  if (iterator.size() == 1) {
    message.cmd.path = {"/"};
  } else {
    // path sanitization
    if (iterator[iterator.size() - 1] == '/') {
      message.failure = Request::Failure::MOVE;
      message.cmd.path = {iterator};
      return message;
    }

    size_t start = 0;
    size_t end = 0;
    while ((end = iterator.find('/', end + 1)) != std::string::npos) {
      message.cmd.path.push_back(iterator.substr(start, end - start));
      start = end;
    }

    message.cmd.path.push_back(iterator.substr(start, end - start));
  }

  ss >> message.cmd.protocol;

  std::string val;

  while (ss >> iterator && ss >> val && iterator != "")
    message.headers[iterator] = val;

  std::unordered_map<std::string, std::string>::const_iterator it =
      message.headers.find("Content-Length:");

  if (it != message.headers.end()) {
    size_t length = atoi(it->second.c_str());

    if (length <= 4096) {
      iterator = _message;

      message.body = iterator.substr(_message.size() - length, length);
    } else {
      message.failure = Request::Failure::LENGTH;
    }
  }

  return message;
};
