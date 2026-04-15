#include <iostream>
#include <vector>

struct WordAndPos {
  std::vector<char> res = {};
  int pos = 0;
};

WordAndPos parser(const std::string &str, int start) {
  WordAndPos wp;
  std::string target = "\r\n";
  size_t delim_pos = str.find(target, start);

  if (delim_pos == std::string::npos)
    return wp;

  for (size_t i = start; i < delim_pos; i++) {
    wp.res.push_back(str[i]);
  }

  wp.pos = delim_pos + 2;
  return wp;
}

int main() {
  std::string str = "*2\r\n$4\r\nECHO\r\n";
  WordAndPos wordAndPos;

  // Keep parsing while we haven't reached the end of the string
  while (wordAndPos.pos < static_cast<int>(str.length())) {
    wordAndPos = parser(str, wordAndPos.pos);

    // If parser returns empty (delimiter not found), we're done
    if (wordAndPos.res.empty()) {
      break;
    }

    // Print the extracted token
    std::cout << "Token: ";
    for (char c : wordAndPos.res) {
      std::cout << c;
    }
    std::cout << "\n";
  }
  std::cout << "this is out of the loop" << "\n";
  for (char c : wordAndPos.res) {
    std::cout << c;
  }

  return 0;
}
