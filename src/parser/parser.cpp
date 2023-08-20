#include "../parser/parser.h"
#include "../request/request.h"
#include <cctype>
#include <cstddef>
#include <cstring>
#include <ctype.h>
#include <iostream>
#include <stdio.h>

int split(char *_p) {
  int i = 0;

  while (memcmp(_p, "\n", 1) != 0 && memcmp(_p, " ", 1) != 0) {
    i++;
    _p++;
  }

  return i;
}

int jump(char *_p) {
  int i = 0;

  while (memcmp(_p, "\n", 1) != 0) {
    i++;
    _p++;
  }

  return i;
}

http::Request http::parse(char *_pBuffer, size_t _size) {
  char *pNext = _pBuffer;
  int i = split(pNext);

  char methodChar[i];
  memcpy(&methodChar, pNext, i);
  methodChar[i] = '\0';

  for (int _i = 0; _i < i; _i++)
    methodChar[_i] = toupper(methodChar[_i]);

  if (strcmp(methodChar, "GET") == 0)
    http::Request::Method method = http::Request::Method::GET;
  else if (strcmp(methodChar, "HEAD") == 0)
    http::Request::Method method = http::Request::Method::HEAD;
  else if (strcmp(methodChar, "POST") == 0)
    http::Request::Method method = http::Request::Method::POST;
  else if (strcmp(methodChar, "PUT") == 0)
    http::Request::Method method = http::Request::Method::PUT;
  else if (strcmp(methodChar, "DELETE") == 0)
    http::Request::Method method = http::Request::Method::DELETE;
  else if (strcmp(methodChar, "CONNECT") == 0)
    http::Request::Method method = http::Request::Method::CONNECT;
  else if (strcmp(methodChar, "OPTIONS") == 0)
    http::Request::Method method = http::Request::Method::OPTIONS;
  else if (strcmp(methodChar, "TRACE") == 0)
    http::Request::Method method = http::Request::Method::TRACE;
  else if (strcmp(methodChar, "PATCH") == 0)
    http::Request::Method method = http::Request::Method::PATCH;
  else
    return http::Request(http::Request::State::ERROR_METHOD,
                         http::Request::Method::PATCH,
                         http::Request::Connection::close, 0.0, "", "");

  pNext += 1 + sizeof(methodChar);
  i = split(pNext);

  char pathChar[i];
  memcpy(&pathChar, pNext, i);
  pathChar[i] = '\0';

  // 1 + 5 for http/
  pNext += 6 + sizeof(pathChar);
  i = split(pNext);

  char versionChar[i];
  memcpy(&versionChar, pNext, i);
  double version = atof(versionChar);

  pNext += 1 + sizeof(versionChar);
  i = jump(pNext);
  pNext += 1 + i;

  // need to jump line

  return http::Request(http::Request::State::ERROR_METHOD,
                       http::Request::Method::PATCH,
                       http::Request::Connection::close, 0.0, "", "");
}
