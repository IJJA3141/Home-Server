#include "./parser.hpp"

#include <cctype>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <ctype.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

std::vector<std::string> split(const char *_pMessage) {
  const char *cursor = _pMessage, *pin = _pMessage;
  std::vector<std::string> vStr;
  size_t size;

  for (int i = 0; i < strlen(_pMessage); i++) {
    if (memcmp(cursor, " ", 1) == 0 || memcmp(cursor, "\n", 1) == 0) {
      size = cursor - pin;
      char str[size];

      memcpy(str, pin, size);
      str[size] = '\0';

      vStr.push_back(std::string(str));

      cursor++;
      pin = cursor;
    } else
      cursor++;
  }

  size = cursor - pin;
  char str[size];

  memcpy(str, pin, size);
  str[size] = '\0';

  vStr.push_back(std::string(str));

  return vStr;
}

http::Method getMethod(std::string _str) {
  if (_str == "GET")
    return http::Method::GET;
  if (_str == "HEAD")
    return http::Method::HEAD;
  if (_str == "POST")
    return http::Method::POST;
  if (_str == "PUT")
    return http::Method::PUT;
  if (_str == "DELETE")
    return http::Method::DELETE;
  if (_str == "OPTIONS")
    return http::Method::OPTIONS;
  if (_str == "TRACE")
    return http::Method::TRACE;
  if (_str == "PATCH")
    return http::Method::PATCH;
  return http::Method::NONE;
}

http::Request::Request(char *_pMessage) {
  std::cout << _pMessage << "\n\n" << std::endl;
  std::vector<std::string> vStr = split(_pMessage);

  for(std::string _str : vStr) {
    std::cout << _str << "\n";
  }

  this->method = getMethod(vStr[0]);
  this->path = vStr[1].c_str();

  for (int i = 4; i < vStr.size(); i++) {
    if (vStr[i] == "Upgrade-Insecure-Requests:") {
      if (vStr[++i] == "1") {
        this->upgrade = true;
      } else {
        this->upgrade = false;
      }
    } else if (vStr[i] == "Content-Length:") {
      this->contentSize = atoi(vStr[++i].c_str());
      this->body = vStr[++i].c_str();
    }
  }

  return;
}
