#include "server/server.h"

int main() {
  Server server;

  if (!server.setup()) {
    return 1;
  }

  server.run();
  return 0;
}
