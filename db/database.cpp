#include "./database.h"

// mixed data structure methods
CompositeValue::CompositeValue(const std::string &str) : value(str) {}
CompositeValue::CompositeValue(const std::vector<std::string> &list)
    : value(list) {}
CompositeValue::CompositeValue(const std::unordered_set<std::string> &set)
    : value(set) {}
CompositeValue::CompositeValue(
    const std::unordered_map<std::string, std::string> &map)
    : value(map) {}

bool CompositeValue::isString() const {
  return std::holds_alternative<std::string>(value);
}
bool CompositeValue::isList() const {
  return std::holds_alternative<std::vector<std::string>>(value);
}
bool CompositeValue::isSet() const {
  return std::holds_alternative<std::unordered_set<std::string>>(value);
}
bool CompositeValue::isMap() const {
  return std::holds_alternative<std::unordered_map<std::string, std::string>>(
      value);
}

std::string &CompositeValue::asString() { return std::get<std::string>(value); }
std::vector<std::string> &CompositeValue::asList() {
  return std::get<std::vector<std::string>>(value);
}
std::unordered_set<std::string> &CompositeValue::asSet() {
  return std::get<std::unordered_set<std::string>>(value);
}
std::unordered_map<std::string, std::string> &CompositeValue::asMap() {
  return std::get<std::unordered_map<std::string, std::string>>(value);
}

// redis command methods
std::string Database::set(const std::string &key, const std::string &value) {
  kv[key] = CompositeValue(value);
  return "+OK\r\r";
}

bool Database::get(const std::string &key, std::string &value) {
  auto it = kv.find(key);
  if (it == kv.end()) {
    return false;
  }

  if (!it->second.isString()) {
    return false;
  }

  value = it->second.asString();
  return true;
}
