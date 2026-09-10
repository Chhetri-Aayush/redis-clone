#ifndef COMMANDHANDLER_H
#define COMMANDHANDLER_H

#include <string>
#include <vector>

class CommandHandler {
public:
  CommandHandler();
  ~CommandHandler();

  bool parseCommand(std::string &input, std::vector<std::string> &tokens);
  std::string executeCommand(const std::vector<std::string> &tokens);
  void processClientInput(std::string &incoming, int clientfd);
};

#endif
