#include "./parser.hpp"

http::Buffer::Buffer(std::string _str) {
#if DEBUG
  std::cout << "New buffer created" << std::endl;
#endif

  this->str_ = _str;
  this->pin_ = -1;
  this->end_ = -1;
  this->next = true;

  return;
}

std::string http::Buffer::GetLine(int _skip) {
  for (int i = 0; i <= _skip; i++) {
    this->pin_ = this->end_ + 1;
    this->end_ = this->str_.find("\n", this->pin_);
  }

  if (this->str_.find("\n", this->end_ + 1) == std::string::npos)
    this->next = false;

  return this->str_.substr(this->pin_, this->end_ - this->pin_);
}

http::Request http::Parse(http::Buffer _buffer) {
  std::cout << "\n___________________\nParsing new message" << std::endl;

  http::Request response;
  std::string str;
  std::string buffer;
  size_t pin;
  size_t pos;
  size_t end;

  str = _buffer.GetLine();

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
  } else
    std::cout << "/!\\ Could not pars method" << std::endl;

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
      response.arg = buffer.substr(++pin, end);
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
  str = _buffer.GetLine();
  pin = str.find(" ") + 1;
  response.header.host = str.substr(pin);

  // agent
  str = _buffer.GetLine();
  pin = str.find(" ") + 1;
  response.header.agent = str.substr(pin);

  // accept
  str = _buffer.GetLine();
  pin = str.find(" ") + 1;
  response.header.accept = str.substr(pin);

  while (_buffer.next) {
    str = _buffer.GetLine();
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
      str = _buffer.GetLine();
      response.header.length =
          std::atoi((str.substr(str.find(" ") + 1)).c_str());
      response.body = _buffer.GetLine(1);
    } else
      std::cout << "Parsing error: " << str << std::endl;
  }

  std::cout << "Method: " << response.method << "\nUri: " << response.uri
            << "\nFile: " << response.file << "\nArg: " << response.arg
            << "\nVersion: " << response.version
            << "\n\nHeader\nHost: " << response.header.host
            << "\nagent: " << response.header.agent
            << "\naccept: " << response.header.accept
            << "\nlang: " << response.header.lang
            << "\nencoding: " << response.header.encoding
            << "\nconnection: " << response.header.connection
            << "\nupgrade: " << response.header.upgrade
            << "\ntype: " << response.header.type
            << "\nlength: " << response.header.length << std::endl;

  return response;
}
