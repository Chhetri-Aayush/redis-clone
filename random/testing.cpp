// #include <arpa/inet.h>
// #include <sys/socket.h>
// #include <unistd.h>
//
// #include <iostream>
//
// int main() {
//   int fd = socket(AF_INET, SOCK_STREAM, 0);
//
//   sockaddr_in addr{};
//   addr.sin_family = AF_INET;
//   addr.sin_port = htons(8080);
//   addr.sin_addr.s_addr = inet_addr("127.0.0.1");
//
//   connect(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
//
//   send(fd, "*3\r\n", 4, 0);
//   sleep(1);
//
//   send(fd, "$3\r\n", 4, 0);
//   sleep(1);
//
//   send(fd, "SET\r\n", 5, 0);
//   sleep(1);
//
//   send(fd, "$4\r\n", 4, 0);
//   sleep(1);
//
//   send(fd, "name\r\n", 6, 0);
//   sleep(1);
//
//   send(fd, "$3\r\n", 4, 0);
//   sleep(1);
//
//   send(fd, "joe\r\n", 5, 0);
//   sleep(1);
//   char buffer[1024];
//
//   ssize_t n = recv(fd, buffer, sizeof(buffer), 0);
//
//   if (n > 0) {
//     std::cout.write(buffer, n);
//   }
//
//   close(fd);
// }
//
//
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>

void sendPart(int fd, const char *data, size_t len) {
  ssize_t n = send(fd, data, len, 0);

  std::cout << "sent " << n << " bytes: [" << data << "]\n";
}

int main() {
  int fd = socket(AF_INET, SOCK_STREAM, 0);

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(8080);
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  if (connect(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
    perror("connect");
    return 1;
  }

  // Read server's welcome message first.
  char buffer[1024];

  ssize_t n = recv(fd, buffer, sizeof(buffer), 0);

  if (n > 0) {
    std::cout << "Welcome: ";
    std::cout.write(buffer, n);
    std::cout << "\n";
  }

  // Now send fragmented RESP.
  sendPart(fd, "*3\r\n", 4);
  sleep(1);

  sendPart(fd, "$3\r\n", 4);
  sleep(1);

  sendPart(fd, "SET\r\n", 5);
  sleep(1);

  sendPart(fd, "$4\r\n", 4);
  sleep(1);

  sendPart(fd, "name\r\n", 6);
  sleep(1);

  sendPart(fd, "$3\r\n", 4);
  sleep(1);

  sendPart(fd, "joe\r\n", 5);

  std::cout << "Finished sending. Waiting for response...\n";

  // Read the response to SET.
  n = recv(fd, buffer, sizeof(buffer), 0);

  std::cout << "recv returned: " << n << "\n";

  if (n > 0) {
    std::cout << "Server response: ";
    std::cout.write(buffer, n);
    std::cout << "\n";
  } else if (n == 0) {
    std::cout << "Server closed connection\n";
  } else {
    perror("recv");
  }

  close(fd);
}
