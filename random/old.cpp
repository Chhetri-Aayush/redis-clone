
#include <atomic>
#include <errno.h>
#include <functional>
#include <iostream>
#include <mutex>
#include <netdb.h>
#include <queue>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>

class Server {
private:
  const char *port;
  int socketfd;
  addrinfo hints{}, *res{};
  std::vector<std::thread> threadPool;
  std::queue<int> clientQueue; // Queue to hold client file descriptors
  std::mutex queueMutex;       // Mutex to protect clientQueue
  std::atomic<bool> running;

public:
  Server(const char *port) : port(port), socketfd(-1), running(true) {}

  // Setup the server
  bool setup() {
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int status = getaddrinfo(NULL, port, &hints, &res);
    if (status != 0) {
      std::cerr << "Error in getaddrinfo: " << gai_strerror(status) << "\n";
      return false;
    }

    // Create socket and bind
    for (addrinfo *p = res; p != NULL; p = p->ai_next) {
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

    if (socketfd == -1) {
      std::cerr << "Failed to bind\n";
      return false;
    }

    if (listen(socketfd, 5) < 0) {
      std::cerr << "Listen error: " << strerror(errno) << "\n";
      return false;
    }

    return true;
  }

  // Worker thread function to handle clients
  void worker() {
    while (running) {
      int clientfd = -1;

      // Wait for work (client connection)
      {
        std::lock_guard<std::mutex> lock(queueMutex);
        if (!clientQueue.empty()) {
          clientfd = clientQueue.front();
          clientQueue.pop();
        }
      }

      // If there is a client to handle, process it
      if (clientfd != -1) {
        std::cout << "Handling client on thread: " << std::this_thread::get_id()
                  << "\n";
        // Simulate work (read, write, etc.)
        close(clientfd); // Close connection after handling
      }
    }
  }

  // Accept connections and push them into the queue for worker threads
  void run() {
    // Start worker threads
    for (int i = 0; i < 4; ++i) { // 4 threads for example
      threadPool.push_back(std::thread(&Server::worker, this));
    }

    while (running) {
      sockaddr_storage clientAddress{};
      socklen_t client_len = sizeof(clientAddress);
      int clientfd =
          accept(socketfd, (struct sockaddr *)&clientAddress, &client_len);

      if (clientfd < 0) {
        std::cerr << "Accept failed: " << strerror(errno) << "\n";
        continue;
      }

      std::cout << "New client connected\n";

      // Push the new client into the queue
      {
        std::lock_guard<std::mutex> lock(queueMutex);
        clientQueue.push(clientfd);
      }
    }

    // Stop the threads when done
    running = false;
    for (auto &thread : threadPool) {
      if (thread.joinable()) {
        thread.join();
      }
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

  server.run(); // Start the server

  return 0;
}
