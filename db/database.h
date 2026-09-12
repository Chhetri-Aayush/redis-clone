#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

enum class ReturnState { Success, NotFound, WrongType };

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
  // common
  ReturnState del(const std::string &key);
  ReturnState exist(const std::string &key);
  // string
  std::string set(const std::string &key, const std::string &value);
  ReturnState get(const std::string &key, std::string &value);
  ReturnState incr(const std::string &key);
  ReturnState decr(const std::string &key);

  // list
  //
  // set
};

#endif
