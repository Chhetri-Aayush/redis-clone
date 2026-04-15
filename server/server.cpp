#include "server.h"
#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

Server::Server() : socketfd(-1) {}

bool Server::setup() {
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

    break;
  }

  freeaddrinfo(res);

  if (p == NULL) {
    std::cerr << "Failed to bind\n";
    return false;
  }

  if (listen(socketfd, 5) < 0) {
    std::cerr << "Listen error\n";
    return false;
  }

  return true;
}

void Server::run() {
  while (true) {
    sockaddr_storage clientAddress{};
    socklen_t client_len = sizeof(clientAddress);

    int clientfd =
        accept(socketfd, (struct sockaddr *)&clientAddress, &client_len);

    if (clientfd < 0) {
      std::cerr << "Accept failed\n";
      continue;
    }

    std::cout << "Client connected\n";

    const char *msg = "welcome to my world\n";

    send(clientfd, msg, strlen(msg), 0);

    char buffer[1024];

    while (true) {
      ssize_t bytes = recv(clientfd, buffer, sizeof(buffer) - 1, 0);

      if (bytes <= 0) {
        std::cout << "Client disconnected\n";
        break;
      }
      buffer[bytes] = '\0';

      std::cout << "the string sent by the user is :" << buffer << "\n";

      if (strcmp(buffer, "PING\r\n") == 0) {
        char send_buffer[] = "+PONG\r\n";
        send(clientfd, send_buffer, strlen(send_buffer), 0);
      } else {
        send(clientfd, buffer, strlen(buffer), 0);
      }
    }

    close(clientfd);
  }
}
Server::~Server() {
  if (socketfd != -1) {
    close(socketfd);
  }
}
