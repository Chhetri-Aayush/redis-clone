#ifndef SERVER_H
#define SERVER_H

#include "../command/commandHandler.h"
#include <netdb.h>

class Server {

private:
  static constexpr const char *port{"8080"};
  int socketfd;
  addrinfo hints{}, *res{}, *p{};
  void handleClient(int clientfd, CommandHandler &cw);
  void processClientInput(std::string &incoming, int clientfd,
                          CommandHandler &cw);

public:
  Server();
  bool setup();
  void run();

  ~Server();
};

#endif
