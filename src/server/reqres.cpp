#include "reqres.hpp"
#include "../log.hpp"

Request::Request()
    : failure(Request::Failure::MALFORMED), connection_type(Client::Type::STANDARD) {};

Request::Request(const std::string _req, const Client::Type _connection_type)
    : connection_type(_connection_type)
{
  Stream stream(_req);
  std::string iterator;
  stream >> iterator;

  this->failure = Request::Failure::NONE;

  switch (iterator.size()) {

  case 3:
    if (iterator.compare(0, 3, "GET") == 0) {
      this->cmd.method = Method::GET;
      break;
    } else if (iterator.compare(0, 3, "POST") == 0) {
      this->cmd.method = Method::PUT;
      break;
    }
  case 4:
    if (iterator.compare(0, 4, "HEAD") == 0) {
      this->cmd.method = Method::HEAD;
      break;
    } else if (iterator.compare(0, 4, "POST") == 0) {
      this->cmd.method = Method::POST;
      break;
    }
  case 5:
    if (iterator.compare(0, 5, "TRACE") == 0) {
      this->cmd.method = Method::TRACE;
      break;
    }
  case 6:
    if (iterator.compare(0, 6, "DELETE") == 0) {
      this->cmd.method = Method::DELETE;
      break;
    }
  case 7:
    if (iterator.compare(0, 7, "CONNECT") == 0) {
      this->cmd.method = Method::CONNECT;
      break;
    } else if (iterator.compare(0, 7, "OPTIONS") == 0) {
      this->cmd.method = Method::OPTIONS;
      break;
    }
  default:
    this->failure = Request::Failure::METHOD;
    LOG("request contains error: METHOD");
    return;
  }

  stream >> iterator;

  if (iterator.size() == 0 || iterator[0] != '/') {
    this->failure = Request::Failure::PATH;
    return;
  }

  if (iterator.size() == 1) {
    this->cmd.path = {"/"};
  } else {
    size_t hash = iterator.find('#');
    size_t quest = iterator.find('?');

    if (hash > quest) hash = quest;

    if (hash != std::string::npos) {
      if (iterator[hash - 1] == '/') {
        this->failure = Request::Failure::TAILING;
        return;
      };

      std::string param = iterator.substr(hash + 1);
      size_t start = 0;

      iterator.resize(hash);

      while ((hash = param.find('=', start)) != std::string::npos) {
        quest = param.find('&');
        this->url_params[param.substr(start, hash)] = param.substr(hash, quest - hash);
        start = quest;
      }

    } else {
      if (iterator[iterator.size() - 1] == '/') {
        this->failure = Request::Failure::TAILING;
        return;
      };
    }

    size_t start = 0;
    size_t end = 0;
    while ((end = iterator.find('/', end + 1)) != std::string::npos) {
      if (start == end + 1) {
        start = end;
        continue;
      }

      this->cmd.path.push_back(iterator.substr(start, end - start));
      start = end;
    }
    this->cmd.path.push_back(iterator.substr(start, end - start));
  }

  stream >> this->cmd.protocol;

  std::string val;
  while (stream >> iterator && stream >> val && iterator != "")
    this->headers[iterator] = val;

  std::map<std::string, std::string>::const_iterator it = this->headers.find("Content-Length:");

  if (it != this->headers.end()) {
    size_t length = atoi(it->second.c_str());

    if (length <= 4096) {
      iterator = _req;

      this->body = iterator.substr(_req.size() - length, length);
    } else {
      LOG("request contains error: SIZE");
      this->failure = Request::Failure::SIZE;
    }
  }

  return;
}
