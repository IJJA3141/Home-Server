#include "reqres.hpp"
#include "../log.hpp"
#include <utility>

bool Stream::operator>>(std::string &_string)
{
  if (this->end_ >= this->npos) return false;

  for (; this->end_ != this->npos; this->end_++) {
    switch (this->string_[this->end_]) {
    case '\n':
      _string = this->string_.substr(this->begin, this->end_ - this->begin);
      this->begin = ++this->end_;
      return true;
    case '\r':
      if (++this->end_ >= this->npos || this->string_[this->end_] == '\n') {
        _string = this->string_.substr(this->begin, --this->end_ - this->begin);
        this->begin = this->end_ += 2;
        return true;
      } else {
        _string = this->string_.substr(this->begin, --this->end_ - this->begin);
        this->begin = ++this->end_;
        return true;
      }
    default:
      break;
    }
  }

  _string = this->string_.substr(this->begin, ++this->end_ - this->begin);

  return true;
};

Request::Request(const Request::Failure _failure) : failure(_failure), ssl(false) {};

Request::Request(const std::string _req, const bool _ssl) : ssl(_ssl)
{
  Stream stream(_req);
  std::string iterator, path;
  size_t pos, npos;
  this->failure = Request::Failure::NONE;

  //// cmd
  // method
  stream >> iterator;
  for (pos = 0; pos < iterator.size(); pos++)
    if (iterator[pos] == ' ') break;

  switch (pos) {
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

  // path
  for (npos = ++pos; npos < iterator.size(); npos++)
    if (iterator[npos] == ' ') break;

  if (pos == npos) {
    WARN("request path failed")
    this->failure = Request::Failure::PATH;
    return;
  }

  path = iterator.substr(pos, npos - pos);

  if (path.size() == 0 || path[0] != '/') {
    this->failure = Request::Failure::PATH;
    return;
  }

  if (path.size() == 1) {
    this->cmd.path = {"/"};
  } else {
    size_t hash = path.find('#');
    size_t quest = path.find('?');

    if (hash > quest) hash = quest;

    if (hash != std::string::npos) {
      if (path[hash - 1] == '/') {
        this->failure = Request::Failure::TRAILING;
        return;
      };

      std::string param = path.substr(hash + 1);
      path.resize(hash);

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
      if (path[path.size() - 1] == '/') {
        this->failure = Request::Failure::TRAILING;
        return;
      };
    }
    size_t start = 0;
    size_t end = 0;
    while ((end = path.find('/', end + 1)) != std::string::npos) {
      if (start == end + 1) {
        start = end;
        continue;
      }

      this->cmd.path.push_back(path.substr(start, end - start));
      start = end;
    }
    this->cmd.path.push_back(path.substr(start, end - start));
  }

  // protocol
  if (++npos >= iterator.length()) {
    this->failure = Request::Failure::PROTOCOL;
    return;
  }

  this->cmd.protocol = iterator.substr(npos);

  // headers
  while (stream >> iterator && iterator != "") {
    pos = iterator.find(':');

    if (pos >= iterator.length() - 2) {
      WARN("request header failed")
      this->failure = Request::Failure::HEADER;
      return;
    }

    this->headers[iterator.substr(0, pos)] = iterator.substr(pos + 2);
  }

  this->body = _req.substr(stream.begin);

  return;
}

std::string Request::to_string() const
{
  std::string res = "";

  switch (this->failure) {
  case NONE:
    res += "Failure: NONE";
    break;
  case METHOD:
    res += "Failure: METHOD";
    break;
  case TRAILING:
    res += "Failure: TRAILING";
    break;
  case SIZE:
    res += "Failure: SIZE";
    break;
  case PATH:
    res += "Failure: PATH";
    break;
  case LENGTH:
    res += "Failure: LENGTH";
    break;
  case UNAUTHORIZEDMETHOD:
    res += "Failure: UNAUTHORIZEDMETHOD";
    break;
  case HEADER:
    res += "Failure: HEADER";
    break;
  case MALFORMED:
    res += "Failure: MALFORMED";
    break;
  case PROTOCOL:
    res += "Failure: PROTOCOL";
    break;
  case WRONGPATH:
    res += "Failure: WRONGPATH";
    break;
  default:
    res += "Failure: fuck";
  }

  res += "\n\n";

  switch (this->cmd.method) {
  case GET:
    res += "GET";
    break;
  case HEAD:
    res += "HEAD";
    break;
  case POST:
    res += "POST";
    break;
  case PUT:
    res += "PUT";
    break;
  case DELETE:
    res += "DELETE";
    break;
  case CONNECT:
    res += "CONNECT";
    break;
  case OPTIONS:
    res += "OPTIONS";
    break;
  case TRACE:
    res += "TRACE";
    break;
  };

  res += " ";

  for (const auto &segment : this->cmd.path)
    res += segment;

  res += " " + this->cmd.protocol + "\n";

  for (const auto &[key, val] : this->headers)
    res += key + ": " + val + "\n";

  res += "\n" + this->body;

  return res;
}

std::string Response::to_string() const
{
  std::string res = "";

  res += this->cmd.protocol + " " + std::to_string(this->cmd.status_code) + "\n";
  res += "content-length: " + std::to_string(this->body.size() + 1) + "\n";

  for (const auto &header : this->headers) {
    res += header.first + ": " + header.second + "\n";
  }

  res += "\n" + this->body + "\n";

  return res;
}
