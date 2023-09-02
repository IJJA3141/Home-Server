#include "./parser.hpp"
#include <cstdlib>
#include <string>

http::Buffer::Buffer(std::string _str) {
  this->str_ = _str;
  this->pin_ = -1;
  this->end_ = -1;
  this->next = true;

  return;
}

std::string http::Buffer::getLine(int _skip) {

  for (int i = 0; i <= _skip; i++) {
    this->pin_ = this->end_ + 1;
    this->end_ = this->str_.find("\n", this->pin_);
  }

  if (this->str_.find("\n", this->end_ + 1) == std::string::npos)
    this->next = false;

  return this->str_.substr(this->pin_, this->end_ - this->pin_);
}

http::Request http::parse(http::Buffer _buffer) {
  http::Request response;
  std::string str;
  std::string buffer;
  size_t pin;
  size_t pos;
  size_t end;

  str = _buffer.getLine();

  pos = str.find(" ");
  buffer = str.substr(0, pos);

  std::transform(buffer.begin(), buffer.end(), buffer.begin(), toupper);
  if (buffer == "GET") {
    response.method = http::Method::GET;
  } else if (buffer == "HEAD") {
    response.method = http::Method::HEAD;
  } else if (buffer == "POST") {
    response.method = http::Method::POST;
  } else if (buffer == "PUT") {
    response.method = http::Method::PUT;
  } else if (buffer == "DELETE") {
    response.method = http::Method::DELETE;
  } else if (buffer == "CONNECT") {
    response.method = http::Method::CONNECT;
  } else if (buffer == "OPTIONS") {
    response.method = http::Method::OPTIONS;
  } else if (buffer == "TRACE") {
    response.method = http::Method::TRACE;
  } else if (buffer == "PATCH") {
    response.method = http::Method::PATCH;
  } else {
    response.method = http::Method::NONE;
  }

  pin = pos;
  pos = str.find_last_of(" ");
  end = pos - 3;
  buffer = str.substr(++pin, end);

  pin = buffer.find_last_of("/");
  pos = buffer.find(".", pin);

  if (pos == std::string::npos) {
    response.uri = buffer;
    response.file = "NULL";
    pos = buffer.find("?");

    if (pos == std::string::npos) {
      response.arg = "NULL";
    } else {
      response.uri = buffer.substr(0, pin);
      response.file = buffer.substr(++pin, end);
    }
  } else {
    response.uri = buffer.substr(0, pin);
    response.file = buffer.substr(++pin, end);

    response.arg = "NULL";
  }

  pin = str.find_last_of(" ");
  response.version = str.substr(++pin);

  // header
  //

  // host
  str = _buffer.getLine();
  pin = str.find(" ") + 1;
  response.header.host = str.substr(pin);

  // agent
  str = _buffer.getLine();
  pin = str.find(" ") + 1;
  response.header.agent = str.substr(pin);

  // accept
  str = _buffer.getLine();
  pin = str.find(" ") + 1;
  response.header.accept = str.substr(pin);

  while (_buffer.next) {
    str = _buffer.getLine();
    if (str.substr(0, str.find(":")) == "Accept-Language") {
      response.header.lang = str.substr(str.find(" ") + 1);
    } else if (str.substr(0, str.find(":")) == "Accept-Encoding") {
      response.header.encoding = str.substr(str.find(" ") + 1);
    } else if (str.substr(0, str.find(":")) == "Connection") {
      buffer = str.substr(str.find(" ") + 1);
      if (buffer == "close") {
        response.header.connection = http::Connection::CLOSE;
      } else if (buffer == "keep-alive") {
        response.header.connection = http::Connection::KEEPALIVE;
      } else
        response.header.connection = http::Connection::ALL;
    } else if (str.substr(0, str.find(":")) == "Upgrade-Insecure-Requests") {
      response.header.upgrade = true;
    } else if (str.substr(0, str.find(":")) == "Content-Type") {
      response.header.type = str.substr(str.find(" ") + 1);
      str = _buffer.getLine();
      response.header.length =
          std::atoi((str.substr(str.find(" ") + 1)).c_str());
      response.body = _buffer.getLine(1);
    } else
      std::cout << "Parsing error" << std::endl;
  }

  return response;
}
