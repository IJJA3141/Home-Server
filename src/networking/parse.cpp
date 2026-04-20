#include "http.hpp"

// helper class
class Iterator
{
public:
  std::string_view model;

  Iterator(const std::string_view _model) : model(_model) {};
  bool operator>>(std::string_view& _view)
  {
    std::size_t pos;
    if ((pos = this->model.find("\r\n")) == this->model.npos || pos == 0) return false;

    _view = this->model.substr(0, pos);
    this->model.remove_prefix(pos + 2);

    return true;
  };
};

http::Request http::parse_request(std::string_view _model)
{
  Iterator it(_model);
  Request request{Error::NONE};

  it >> _model;

  std::size_t pos = _model.find(' ');
  if (pos == _model.npos || parse_method(_model.substr(0, pos), request.cmd.method)) return {Error::M_METHOD};
  _model.remove_prefix(pos + 1);

  pos = _model.find(' ');
  if (pos == _model.npos || parse_url(_model.substr(0, pos), request.cmd.url)) return {Error::M_URL};

  if (parse_protocol(_model.substr(pos + 1), request.cmd.protocol)) return {Error::M_PROTOCOL};

  while (it >> _model)
  {
    if ((pos = _model.find(':')) == _model.npos) return {Error::M_HEADER};
    if (_model.substr(0, pos) == "")
    {
    }
    else
    {
      // request.headers[std::string(_model.substr(0, pos))] = std::string(_model.substr(pos + 2));
      // request.headers[std::string(_model.substr(0, pos))] = std::string(_model.substr(pos + 2));
    }
  }

  it.model.remove_prefix(2);
  request.body = it.model;
  return request;
}

http::Response http::parse_response(std::string_view _model)
{
  Iterator it(_model);
  Response response{};

  response.body = _model;

  return response;
}

bool http::parse_method(const std::string_view _model, http::Method& _method)
{
  switch (_model[0])
  {
  case 'G':
    if (_model != "GET") return true;
    _method = Method::GET;
    return false;

  case 'H':
    if (_model != "HEAD") return true;
    _method = Method::HEAD;
    return false;

  case 'D':
    if (_model != "DELETE") return true;
    _method = Method::DELETE;
    return false;

  case 'C':
    if (_model != "CONNECT") return true;
    _method = Method::CONNECT;
    return false;

  case 'O':
    if (_model != "OPTIONS") return true;
    _method = Method::OPTIONS;
    return false;

  case 'T':
    if (_model != "TRACE") return true;
    _method = Method::TRACE;
    return false;

  case 'P':
    if (_model == "PUT")
    {
      _method = Method::PUT;
      return false;
    }

    if (_model == "POST")
    {
      _method = Method::POST;
      return false;
    }
  }

  return true;
}

bool http::parse_url(std::string_view _model, http::Url& _url)
{
  std::size_t pos;

  if ((pos = _model.find_last_of('#')) != _model.npos)
  {
    _url.fragment = std::string(_model.substr(pos + 1));
    _model.remove_suffix(_model.size() - pos);
  }

  if ((pos = _model.find_last_of('?')) != _model.npos)
  {
    if (parse_querys(_model.substr(pos + 1), _url.querys)) return true;
    _model.remove_suffix(_model.size() - pos);
  }

  if (_model[0] != '/') return true;
  _url.path = std::string(_model);

  return false;
}

bool http::parse_querys(std::string_view _model, std::map<std::string, std::string>& _querys)
{
  std::size_t pos, eq;

  while ((pos = _model.find_last_of('&')) != _model.npos)
  {
    if ((eq = _model.find_last_of('=')) == _model.npos) return true;

    _querys[std::string(_model.substr(pos + 1, eq - pos - 1))] = std::string(_model.substr(eq + 1));
    _model.remove_suffix(_model.size() - pos);
  }

  if ((eq = _model.find_last_of('=')) == _model.npos) return true;
  _querys[std::string(_model.substr(0, eq))] = std::string(_model.substr(eq + 1));

  return false;
}

bool http::parse_protocol(std::string_view _model, Protocol& _protocol)
{
  if (_model == "HTTP/0.9")
  {
    _protocol = Protocol::HTTP_09;
    return false;
  }

  if (_model == "HTTP/1.0")
  {
    _protocol = Protocol::HTTP_10;
    return false;
  }

  if (_model == "HTTP/1.1")
  {
    _protocol = Protocol::HTTP_11;
    return false;
  }

  if (_model == "HTTP/2.0")
  {
    _protocol = Protocol::HTTP_20;
    return false;
  }

  if (_model == "HTTP/3.0")
  {
    _protocol = Protocol::HTTP_30;
    return false;
  }

  return true;
}

bool http::parse_cookies(std::string_view _model, std::map<std::string, std::string>& _cookies)
{
  // 6 cookie, 2 : , 3 <...>=<...>
  if (_model.size() < 8 + 3 || _model.substr(0, 8) != "Cookie: ") return true;
  _model.remove_prefix(8);

  std::size_t eq_pos, end_pos;
  while (!_model.empty())
  {
    eq_pos = _model.find("=");
    end_pos = _model.find(";");

    _cookies.emplace(_model.substr(0, eq_pos), _model.substr(eq_pos + 1, end_pos - eq_pos - 1));

    if (end_pos == _model.npos) break;
    _model.remove_prefix(std::min(end_pos + 2, _model.size()));
  }

  return false;
}
