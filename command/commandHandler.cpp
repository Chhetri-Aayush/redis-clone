#include "./commandHandler.h"
#include <iostream>
#include <sys/socket.h>
#include <vector>

static const size_t MaxRequestLen = 4096;
static const int MaxArguments = 128;

CommandHandler::CommandHandler() {}
CommandHandler::~CommandHandler() {}

// std::vector<std::string>
// CommandHandler::parseCommand(const std::string &input) {
//   std::vector<std::string> tokens;
//   if (input.empty())
//     return tokens;
//
//   if (input[0] != '*') {
//     std::istringstream iss(input);
//     std::string token;
//     while (iss >> token)
//       tokens.push_back(token);
//     return tokens;
//   }
//
//   size_t pos = 0;
//
//   if (input[pos] != '*')
//     return tokens;
//   pos++;
//
//   size_t delim_pos = input.find("\r\n", pos);
//   if (delim_pos == std::string::npos)
//     return tokens;
//
//   int numElements = std::stoi(input.substr(pos, delim_pos - pos));
//   pos = delim_pos + 2;
//
//   for (int i = 0; i < numElements; i++) {
//     if (pos >= input.size() || input[pos] != '$')
//       break;
//
//     pos++;
//
//     delim_pos = input.find("\r\n", pos);
//     if (delim_pos == std::string::npos)
//       break;
//
//     int len = std::stoi(input.substr(pos, delim_pos - pos));
//     pos = delim_pos + 2;
//
//     if (pos + len > input.size())
//       break;
//
//     std::string token = input.substr(pos, len);
//     tokens.push_back(token);
//
//     pos += len + 2;
//   }
//
//   return tokens;
// }

bool CommandHandler::parseCommand(std::string &input,
                                  std::vector<std::string> &tokens) {
  tokens.clear();

  if (input.empty()) {
    return false;
  }

  if (input[0] != '*') {
    std::cerr << "Invalid RESP: expected '*'\n";
    input.clear();
    return false;
  }

  size_t pos = 0;

  if (input[pos] != '*') {
    return false;
  }

  pos++;
  size_t delim_pos = input.find("\r\n", pos);

  if (delim_pos == std::string::npos) {
    return false;
  }

  int numElements;

  try {
    numElements = std::stoi(input.substr(pos, delim_pos - pos));
  } catch (...) {
    std::cerr << "Invalid RESP array length\n";
    input.clear();
    return false;
  }

  if (numElements <= 0 || numElements > MaxArguments) {
    std::cerr << "Invalid number of arguments\n";
    input.clear();
    return false;
  }

  pos = delim_pos + 2;

  for (int i = 0; i < numElements; ++i) {
    if (pos >= input.size()) {
      return false;
    }

    if (input[pos] != '$') {
      std::cerr << "Invalid RESP: expected '$'\n";
      input.clear();
      return false;
    }

    pos++;

    delim_pos = input.find("\r\n", pos);

    if (delim_pos == std::string::npos) {
      return false;
    }

    int len;

    try {
      len = std::stoi(input.substr(pos, delim_pos - pos));
    } catch (...) {
      std::cerr << "Invalid bulk string length\n";
      input.clear();
      return false;
    }

    if (len < 0 || static_cast<size_t>(len) > MaxRequestLen) {
      std::cerr << "Invalid bulk string length\n";
      input.clear();
      return false;
    }

    pos = delim_pos + 2;

    if (input.size() < pos + static_cast<size_t>(len) + 2) {
      return false;
    }

    std::string token = input.substr(pos, len);
    tokens.push_back(token);

    pos += len;

    if (input[pos] != '\r' || input[pos + 1] != '\n') {
      std::cerr << "Invalid RESP: missing CRLF\n";
      input.clear();
      return false;
    }

    pos += 2;
  }

  input.erase(0, pos);

  return true;
}

std::string
CommandHandler::executeCommand(const std::vector<std::string> &tokens) {
  if (tokens.size() == 0) {
    return "-Err: empty command\r\n";
  }

  std::string cmd = tokens[0];

  return "-Err: unknown command\r\r";
}

void CommandHandler::processClientInput(std::string &incoming, int clientfd) {
  while (true) {
    std::vector<std::string> tokens;
    bool complete = parseCommand(incoming, tokens);

    if (!complete) {
      return;
    }
    std::cout << "Command received:\n";
    for (const auto &token : tokens) {
      std::cout << "  [" << token << "]\n";
    }
    const std::string output = executeCommand(tokens);
    ssize_t sent = send(clientfd, output.data(), output.size(), 0);

    if (sent < 0) {
      std::cerr << "send failed\n";
      return;
    }
  }
}
