#include "static.hpp"
#include "../file/loading.hpp"

Response read_js(Request _request)
{
  Response res;

  try {
    res.cmd = {"HTTP/1.1", 200};
    res.body = Loader::load_file("static/js/" + _request.url_args["file"]);
  } catch (std::exception _error) {
    res.cmd.status_code = 404;
    return res;
  } catch (Loader::exception _error) {
    res.cmd.status_code = 404;
    return res;
  };

  res.headers["Content-type"] = "text/javascript";

  return res;
}

Response read_css(Request _request)
{
  Response res;

  try {
    res.cmd = {"HTTP/1.1", 200};
    res.body = Loader::load_file("static/css/" + _request.url_args["file"]);
  } catch (std::exception _error) {
    res.cmd.status_code = 404;
    return res;
  } catch (Loader::exception _error) {
    res.cmd.status_code = 404;
    return res;
  };

  res.headers["Content-type"] = "text/css";

  return res;
}
