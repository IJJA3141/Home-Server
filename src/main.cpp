#include "server/server.hpp"

int main(int _argc, char *_argv[])
{
  Router prs(_argv[0]);

  Tcp server(&prs);
  server.bind(80);
  server.listen();

  parser.add(GET, "/over/there", [](){}, Client::Type::SSL);

  return 0;
}
