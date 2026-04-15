#include "server/server.h"

int main(int argc, char *argv[]) {
  Server server;

  if (!server.setup()) {
    return 1;
  }

  server.run();
  return 0;
}
