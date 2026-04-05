// #include <iostream>
// #include <netdb.h>
// #include <string.h>
// #include <sys/socket.h>
// #include <sys/types.h>
// #include <unistd.h>
//
// int main() {
//   constexpr const char *MYPORT = "8080";
//
//   addrinfo hints{};
//   addrinfo *res{}, *p{};
//   int socketfd{};
//
//   hints.ai_family = AF_UNSPEC;
//   hints.ai_socktype = SOCK_STREAM;
//   hints.ai_flags = AI_PASSIVE;
//
//   int status = getaddrinfo(NULL, MYPORT, &hints, &res);
//   if (status != 0) {
//     std::cerr << "There is some problem while creating the addresses: "
//               << gai_strerror(status) << "\n";
//     return 1;
//   }
//
//   for (p = res; p != NULL; p = p->ai_next) {
//     socketfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
//     if (socketfd == -1) {
//       continue;
//     }
//
//     int yes = 1;
//     setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
//
//     if (bind(socketfd, p->ai_addr, p->ai_addrlen) == -1) {
//       close(socketfd);
//       continue;
//     }
//     std::cout << "the choosen address is :" << p->ai_family << "\n";
//     break;
//   }
//
//   freeaddrinfo(res);
//
//   if (p == NULL) {
//     std::cerr << "failed to bind to any address\n";
//     return 1;
//   }
//   if (listen(socketfd, 5) < 0) {
//
//     std::cerr << "three is some problem while listening: " << strerror(errno)
//               << "\n";
//     return 1;
//   }
//
//   while (true) {
//
//     sockaddr_storage clientAddress{};
//     socklen_t client_len = sizeof(clientAddress);
//
//     int clientfd =
//         accept(socketfd, (struct sockaddr *)&clientAddress, &client_len);
//
//     if (clientfd < 0) {
//       std::cerr << "accept failed: " << strerror(errno) << "\n";
//       continue;
//     }
//     std::cout << "client connected" << "\n";
//     close(clientfd);
//   }
//   close(socketfd);
//   return 0;
// }
//
//
