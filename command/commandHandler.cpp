#include "./commandHandler.h"
#include <sstream>
#include <vector>

CommandHandler::CommandHandler() {}
CommandHandler::~CommandHandler() {}

std::vector<std::string>
CommandHandler::parseCommand(const std::string &input) {
  std::vector<std::string> tokens;
  if (input.empty())
    return tokens;

  if (input[0] != '*') {
    std::istringstream iss(input);
    std::string token;
    while (iss >> token)
      tokens.push_back(token);
    return tokens;
  }

  size_t pos = 0;

  if (input[pos] != '*')
    return tokens;
  pos++;

  size_t delim_pos = input.find("\r\n", pos);
  if (delim_pos == std::string::npos)
    return tokens;

  int numElements = std::stoi(input.substr(pos, delim_pos - pos));
  pos = delim_pos + 2;

  for (int i = 0; i < numElements; i++) {
    if (pos >= input.size() || input[pos] != '$')
      break;

    pos++;

    delim_pos = input.find("\r\n", pos);
    if (delim_pos == std::string::npos)
      break;

    int len = std::stoi(input.substr(pos, delim_pos - pos));
    pos = delim_pos + 2;

    if (pos + len > input.size())
      break;

    std::string token = input.substr(pos, len);
    tokens.push_back(token);

    pos += len + 2;
  }

  return tokens;
}

std::string
CommandHandler::executeCommand(const std::vector<std::string> &tokens) {
  if (tokens.size() == 0) {
    return "-Err: empty command\r\n";
  }

  std::string cmd = tokens[0];

  return "-Err: unknown command\r\r";
}
