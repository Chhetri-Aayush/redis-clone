#ifndef COMMANDHANDLER_H
#define COMMANDHANDLER_H

#include "../db/database.h"
#include <string>
#include <vector>

class CommandHandler {
private:
  Database db;

public:
  CommandHandler();
  ~CommandHandler();

  bool parseCommand(std::string &input, std::vector<std::string> &tokens);
  std::string executeCommand(const std::vector<std::string> &tokens);
  void processClientInput(std::string &incoming, int clientfd);

  bool loadPersistedData() { return db.loadFromDisk("dump.rdb"); }
  bool persistData() { return db.saveToDisk("dump.rdb"); }
};

#endif
