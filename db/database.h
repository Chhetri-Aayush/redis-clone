#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

class CompositeValue {
private:
  std::variant<std::string, std::vector<std::string>,
               std::unordered_set<std::string>,
               std::unordered_map<std::string, std::string>>
      value;

public:
  CompositeValue(const std::string &str);
  CompositeValue(const std::vector<std::string> &list);
  CompositeValue(const std::unordered_set<std::string> &set);
  CompositeValue(const std::unordered_map<std::string, std::string> &map);

  bool isString() const;
  bool isList() const;
  bool isSet() const;
  bool isMap() const;

  std::string &asString();
  std::vector<std::string> &asList();
  std::unordered_set<std::string> &asSet();
  std::unordered_map<std::string, std::string> &asMap();
};

class Database {
private:
  std::unordered_map<std::string, CompositeValue> kv;

public:
  std::string set(const std::string &key, const std::string &value);
  bool get(const std::string &key, std::string &value);
};

#endif
