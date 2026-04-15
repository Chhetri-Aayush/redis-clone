#ifndef COMMANDHANDLER_H
#define COMMANDHANDLER_H

#include <string>

class CommandHanlder {
public:
  CommandHanlder();
  ~CommandHanlder();

  std::string processCommand(const std::string &input);
};

#endif
