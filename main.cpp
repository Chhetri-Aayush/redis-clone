#include <arpa/inet.h>
#include <iostream>
#include <string.h>
#include <sys/socket.h>

int main() {
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    std::cerr << "Problem while creation of the socket: " << strerror(errno)
              << "\n";
  }

  sockaddr_in serverAddress;
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(8080);
  serverAddress.sin_addr.s_addr = INADDR_ANY;

  if (bind(sockfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) <
      0) {
    std::cerr << "three is some problem while binding: " << strerror(errno)
              << "\n";
  }

  listen(sockfd, 5);

  return 0;
}
