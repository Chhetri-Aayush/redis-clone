#include "server.h"
#include "../command/commandHandler.h"
#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

static const size_t MaxLineLen = 4096;

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

    int val = 1;
    setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

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

  std::cout << " the setup is good and we are procedding to the run"
            << std::endl;
  return true;
}

// void Server::run() {
//   CommandHandler cw;
//   while (true) {
//     sockaddr_storage clientAddress{};
//     socklen_t client_len = sizeof(clientAddress);
//
//     int clientfd =
//         accept(socketfd, (struct sockaddr *)&clientAddress, &client_len);
//
//     if (clientfd < 0) {
//       std::cerr << "Accept failed\n";
//       continue;
//     }
//
//     std::cout << "Client connected\n";
//     const char *msg = "welcome to my world\n";
//     send(clientfd, msg, strlen(msg), 0);
//     // char buffer[1024];
//     std::string buffer;
//
//     while (true) {
//       buffer.resize(1024);
//       // ssize_t bytes = recv(clientfd, buffer, sizeof(buffer) - 1, 0);
//       ssize_t bytes = recv(clientfd, buffer.data(), buffer.size(), 0);
//
//       if (bytes <= 0) {
//         std::cout << "Client disconnected\n";
//         break;
//       }
//
//       buffer.resize(bytes);
//       // buffer[bytes] = '\0';
//       std::cout << "the string sent by the user is :" << buffer << "\n";
//
//       // std::string input(buffer);
//       const std::vector<std::string> &tokens = cw.parseCommand(buffer);
//       const std::string output = cw.executeCommand(tokens);
//
//       const char *outStr = output.c_str();
//       // send(clientfd, outStr, strlen(outStr), 0);
//       send(clientfd, output.data(), output.size(), 0);
//
//       // if (strcmp(buffer, "PING\r\n") == 0) {
//       //   char send_buffer[] = "+PONG\r\n";
//       //   send(clientfd, send_buffer, strlen(send_buffer), 0);
//       // } else {
//       //   send(clientfd, buffer, strlen(buffer), 0);
//       // }
//     }
//
//     close(clientfd);
//   }
// }
void Server::run() {
  CommandHandler cw;
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
    const char *welcome = "welcome to my world\n";
    send(clientfd, welcome, strlen(welcome), 0);

    handleClientInput(clientfd, cw);

    close(clientfd);
  }
}

void Server::handleClientInput(int clientfd, CommandHandler &cw) {
  std::string input;
  char recvBuf[1024];

  while (true) {
    ssize_t bytes = recv(clientfd, recvBuf, sizeof(recvBuf), 0);

    if (bytes <= 0) {
      std::cout << "Client disconnected\n";
      return;
    }
    input.append(recvBuf, bytes);

    if (input.size() > MaxLineLen) {
      std::cerr << "Line too long, dropping client\n";
      return;
    }

    cw.processClientInput(input, clientfd);
  }
}

Server::~Server() {
  if (socketfd != -1) {
    close(socketfd);
  }
}
