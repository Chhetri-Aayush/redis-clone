#ifndef COMMANDHANDLER_H
#define COMMANDHANDLER_H

#include <string>
#include <vector>

class CommandHandler {
public:
  CommandHandler();
  ~CommandHandler();

  std::vector<std::string> parseCommand(const std::string &input);
  std::string executeCommand(const std::vector<std::string> &tokens);
};

#endif
