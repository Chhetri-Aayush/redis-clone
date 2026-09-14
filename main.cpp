#include "server/server.h"
#include <csignal>
#include <cstdlib>

// CommandHandler *globalHandlerPtr = nullptr;
//
// void handleShutdown(int) {
//   if (globalHandlerPtr) {
//     globalHandlerPtr->persistData();
//   }
//   std::exit(0);
// }

int main(int argc, char *argv[]) {
  Server server;

  if (!server.setup()) {
    return 1;
  }
  // globalHandlerPtr = &server.getCommandHandler();
  // std::signal(SIGINT, handleShutdown);
  // std::signal(SIGTERM, handleShutdown);

  server.run();
  return 0;
}
