#include <iostream>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

class Server {
private:
  const char *port;
  int socketfd;
  addrinfo hints{}, *res{}, *p{};

public:
  Server(const char *port) : port(port), socketfd(-1) {}

  bool setup() {
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int status = getaddrinfo(NULL, port, &hints, &res);
    if (status != 0) {
      std::cerr << "Error in getaddrinfo: " << gai_strerror(status) << "\n";
      return false;
    }

    for (p = res; p != NULL; p = p->ai_next) {
      socketfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
      if (socketfd == -1)
        continue;

      int yes = 1;
      setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

      if (bind(socketfd, p->ai_addr, p->ai_addrlen) == -1) {
        close(socketfd);
        continue;
      }

      std::cout << "Chosen address family: " << p->ai_family << "\n";
      break;
    }

    freeaddrinfo(res);

    if (p == NULL) {
      std::cerr << "Failed to bind\n";
      return false;
    }

    if (listen(socketfd, 5) < 0) {
      std::cerr << "Listen error: " << strerror(errno) << "\n";
      return false;
    }

    return true;
  }

  void run() {
    while (true) {
      sockaddr_storage clientAddress{};
      socklen_t client_len = sizeof(clientAddress);

      int clientfd =
          accept(socketfd, (struct sockaddr *)&clientAddress, &client_len);

      if (clientfd < 0) {
        std::cerr << "Accept failed: " << strerror(errno) << "\n";
        continue;
      }

      std::cout << "Client connected\n";
      close(clientfd);
    }
  }

  ~Server() {
    if (socketfd != -1) {
      close(socketfd);
    }
  }
};

int main() {
  Server server("8080");

  if (!server.setup()) {
    return 1;
  }

  server.run();
  return 0;
}
