// #include <arpa/inet.h>
// #include <errno.h>
// #include <iostream>
// #include <string.h>
// #include <sys/socket.h>
// #include <unistd.h>
//
// int main() {
//   int sockfd = socket(AF_INET, SOCK_STREAM, 0);
//   if (sockfd == -1) {
//     std::cerr << "Problem while creation of the socket: " << strerror(errno)
//               << "\n";
//     return 1;
//   }
//
//   sockaddr_in serverAddress{};
//
//   serverAddress.sin_family = AF_INET;
//   serverAddress.sin_port = htons(8080);
//   serverAddress.sin_addr.s_addr = INADDR_ANY;
//
//   if (bind(sockfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress))
//   <
//       0) {
//     std::cerr << "three is some problem while binding: " << strerror(errno)
//               << "\n";
//     return 1;
//   }
//
//   if (listen(sockfd, 5) < 0) {
//
//     std::cerr << "three is some problem while listening: " << strerror(errno)
//               << "\n";
//     return 1;
//   }
//
//   while (true) {
//
//     sockaddr_in clientAddress{};
//     socklen_t client_len = sizeof(clientAddress);
//
//     int clientfd =
//         accept(sockfd, (struct sockaddr *)&clientAddress, &client_len);
//
//     if (clientfd < 0) {
//       std::cerr << "accept failed: " << strerror(errno) << "\n";
//     }
//     std::cout << "client connected" << "\n";
//     close(clientfd);
//   }
//   close(sockfd);
//
//   return 0;
// }
