#ifndef SERVER_H
#define SERVER_H

#include <netdb.h>

class Server {
private:
  static constexpr const char *port{"8080"};
  int socketfd;
  addrinfo hints{}, *res{}, *p{};

public:
  Server();
  bool setup();
  void run();
  ~Server();
};

#endif
