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

Request parse_request(const std::string_view _model)
{
  Iterator it(_model);
  std::string_view view;
  Request request;

  it >> view;

  std::size_t pos = view.find(' ');
  if (pos == view.npos || parse_method(view.substr(0, pos), request.cmd.method)) return {Request::M_METHOD};

  view.remove_prefix(pos + 1);
  pos = view.find(' ');
  if (pos == view.npos || parse_url(view.substr(0, pos), request.cmd.url)) return {Request::M_URL};

  request.cmd.protocol = std::string(view.substr(pos + 1));

  while (it >> view)
  {
    if ((pos = view.find(':')) == view.npos) return {Request::M_HEADER};
    request.headers[std::string(view.substr(0, pos))] = std::string(view.substr(pos + 2));
  }

  it.model.remove_prefix(2);
  request.body = it.model;
  return request;
}

bool parse_method(const std::string_view _model, Method& _method)
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

bool parse_url(std::string_view _model, Url& _url)
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
  _model.remove_prefix(1);

  while ((pos = _model.find('/')) != _model.npos)
  {
    _url.path.push_back(std::string(_model.substr(0, pos)));
    _model.remove_prefix(pos + 1);
  }

  _url.path.push_back(std::string(_model));
  return false;
}

bool parse_querys(std::string_view _model, std::map<std::string, std::string>& _querys)
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

Response::operator const std::string() const
{
  std::stringstream ss;

  ss << this->protocol << " ";
  ss << this->code << " ";
  ss << this->message << "\r\n";

  for (const auto& header : this->headers)
    ss << header.first << ": " << header.second << "\r\n";

  ss << "\r\n" << this->body;

  return ss.str();
};
