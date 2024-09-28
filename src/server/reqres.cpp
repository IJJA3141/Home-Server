#include "reqres.hpp"
#include "../log.hpp"

bool Stream::operator>>(std::string &_string)
{
  if (this->end_ >= this->npos) return false;

  for (; this->end_ != this->npos; this->end_++) {
    switch (this->string_[this->end_]) {
    case ' ':
    case '\n':
      _string = this->string_.substr(this->begin_, this->end_ - this->begin_);
      this->begin_ = ++this->end_;
      return true;
    case '\r':
      if (++this->end_ >= this->npos || this->string_[this->end_] == '\n') {
        _string = this->string_.substr(this->begin_, --this->end_ - this->begin_);
        this->begin_ = this->end_ += 2;
        return true;
      } else {
        _string = this->string_.substr(this->begin_, --this->end_ - this->begin_);
        this->begin_ = ++this->end_;
        return true;
      }
    default:
      break;
    }
  }

  _string = this->string_.substr(this->begin_, ++this->end_ - this->begin_);

  return true;
};

Request::Request(const Request::Failure _failure)
    : failure(_failure), connection_type(Client::Type::STANDARD){};

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
        this->failure = Request::Failure::TRAILING;
        return;
      };

      std::string param = iterator.substr(hash + 1);
      iterator.resize(hash);

      size_t start = 0;
      hash = 0;
      quest = 0;

      hash = param.find('=');
      quest = param.find('&');

      while (quest != std::string::npos) {
        this->url_params[param.substr(start, hash - start)] =
            param.substr(hash + 1, quest - hash - 1);

        start = quest + 1;

        hash = param.find('=', start);
        quest = param.find('&', hash);
      }

      this->url_params[param.substr(start, hash - start)] = param.substr(hash + 1);

    } else {
      if (iterator[iterator.size() - 1] == '/') {
        this->failure = Request::Failure::TRAILING;
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
  while (stream >> iterator && stream >> val && iterator != "") {
    iterator.pop_back();
    this->headers[iterator] = val;
  }

  this->body = val;

  /*
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
  */

  return;
}

std::string Response::to_string() const
{
  std::string res = "";

  res += this->cmd.protocol + " " + std::to_string(this->cmd.status_code) + "\n";
  res += "content-length: " + std::to_string(this->body.size() + 1) + "\n";

  PRINT(this->headers.size());

  for (const auto &header : this->headers) {
    PRINT("ALO");
    res += header.first + ": " + header.second + "\n";
  }

  res += "\n" + this->body + "\n";

  return res;
}
