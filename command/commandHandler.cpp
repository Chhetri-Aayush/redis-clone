#include "./commandHandler.h"
#include "../db/database.h"
#include <iostream>
#include <sys/socket.h>
#include <vector>

namespace {

const size_t MaxRequestLen = 4096;
const int MaxArguments = 128;

Database db;

std::string handlePing(const std::vector<std::string> &tokens) {
  return "+PONG\r\n";
}

std::string handleSET(const std::vector<std::string> &tokens) {
  if (tokens.size() < 3)
    return "-Err:SET requires key and value\r\n";
  // return db.set(tokens[1], tokens[2]);
  ReturnState state = db.set(tokens[1], tokens[2]);

  switch (state) {
  case ReturnState::Success:
    return "+OK\r\n";
  // case ReturnState::WrongType:
  //   return "-WRONGTYPE Operation against a key holding the wrong kind of "
  //          "value\r\n";
  default:
    return "-ERR unknown error\r\n";
  }
}

std::string handleGET(const std::vector<std::string> &tokens) {
  if (tokens.size() < 2)
    return "-ERR GET requires a key\r\n";

  std::string value;
  ReturnState state = db.get(tokens[1], value);

  switch (state) {
  case ReturnState::Success:
    return "$" + std::to_string(value.size()) + "\r\n" + value + "\r\n";
  case ReturnState::NotFound:
    return "$-1\r\n";
  case ReturnState::WrongType:
    return "-WRONGTYPE Operation against a key holding the wrong kind of "
           "value\r\n";
  default:
    return "-ERR unknown error\r\n";
  }
}

std::string handleDEL(const std::vector<std::string> &tokens) {
  if (tokens.size() < 2)
    return "-ERR wrong number of arguments for 'del' command\r\n";

  ReturnState state = db.del(tokens[1]);
  switch (state) {
  case ReturnState::Success:
    return ":1\r\n";
  case ReturnState::NotFound:
    return ":0\r\n";
  default:
    return "-ERR unknown error\r\n";
  }
}

std::string handleEXIST(const std::vector<std::string> &tokens) {
  if (tokens.size() < 2)
    return "-ERR wrong number of arguments for 'exists' command\r\n";

  ReturnState state = db.exist(tokens[1]);
  switch (state) {
  case ReturnState::Success:
    return ":1\r\n";
  case ReturnState::NotFound:
    return ":0\r\n";
  default:
    return "-ERR unknown error\r\n";
  }
}

} // namespace

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
  if (cmd == "PING") {
    return handlePing(tokens);
  } else if (cmd == "SET") {
    return handleSET(tokens);
  } else if (cmd == "GET") {
    return handleGET(tokens);
  } else if (cmd == "EXIST") {
    return handleEXIST(tokens);
  } else if (cmd == "DEL") {
    return handleDEL(tokens);
  } else {
    return "-Err: unknown command\r\n";
  }
}

void CommandHandler::processClientInput(std::string &input, int clientfd) {
  while (true) {
    std::vector<std::string> tokens;
    bool complete = parseCommand(input, tokens);

    if (!complete) {
      // std::cout << " the message is not complete will try again whne the new
      // "
      //              "message arrives and the command is actually there  "
      //           << "\n";
      // std::cout << " so far the incoming consist of " << input << "\n";
      return;
    }
    // std::cout << "Command received:\n";
    // for (const auto &token : tokens) {
    //   std::cout << "  [" << token << "]\n";
    // }
    const std::string output = executeCommand(tokens);
    ssize_t sent = send(clientfd, output.data(), output.size(), 0);

    if (sent < 0) {
      std::cerr << "send failed\n";
      return;
    }
  }
}
