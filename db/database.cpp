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

bool Database::isExpired(const std::string &key) {
  auto it = kv.find(key);

  if (it == kv.end()) {
    return false;
  }
  StoredValue &stored = it->second;
  if (!stored.hasTTL) {
    return false;
  }
  if (Clock::now() >= stored.expiresAt) {
    kv.erase(it);
    return true;
  }

  return false;
}

// redis command methods
ReturnState Database::set(const std::string &key, const std::string &value) {
  // kv[key] = CompositeValue(value);
  kv[key] = StoredValue{CompositeValue(value), false, TimePoint{}};
  return ReturnState::Success;
}

ReturnState Database::set(const std::string &key, const std::string &value,
                          std::int64_t ttlSeconds) {
  if (ttlSeconds <= 0) {
    return ReturnState::InvalidValue;
  }

  kv[key] = StoredValue{CompositeValue(value), true,
                        Clock::now() + std::chrono::seconds(ttlSeconds)};
  return ReturnState::Success;
}

// bool Database::get(const std::string &key, std::string &value) {
//   auto it = kv.find(key);
//   if (it == kv.end()) {
//     return false;
//   }
//
//   if (!it->second.isString()) {
//     return false;
//   }
//
//   value = it->second.asString();
//   return true;
// }
ReturnState Database::get(const std::string &key, std::string &value) {
  if (isExpired(key)) {
    return ReturnState::NotFound;
  }
  auto it = kv.find(key);

  if (it == kv.end()) {
    return ReturnState::NotFound;
  }

  if (!it->second.data.isString()) {
    return ReturnState::WrongType;
  }
  value = it->second.data.asString();
  return ReturnState::Success;
}

ReturnState Database::del(const std::string &key) {
  if (isExpired(key)) {
    return ReturnState::NotFound;
  }
  auto it = kv.find(key);

  if (it == kv.end()) {
    return ReturnState::NotFound;
  }

  kv.erase(it);
  return ReturnState::Success;
}

ReturnState Database::exist(const std::string &key) {
  if (isExpired(key)) {
    return ReturnState::NotFound;
  }

  auto it = kv.find(key);
  if (it == kv.end()) {
    return ReturnState::NotFound;
  }
  return ReturnState::Success;
}

ReturnState Database::expire(const std::string &key, std::int64_t seconds) {
  if (isExpired(key)) {
    return ReturnState::NotFound;
  }

  auto it = kv.find(key);

  if (it == kv.end()) {
    return ReturnState::NotFound;
  }

  if (seconds <= 0) {
    kv.erase(it);
    return ReturnState::Success;
  }

  it->second.hasTTL = true;
  it->second.expiresAt = Clock::now() + std::chrono::seconds(seconds);

  return ReturnState::Success;
}

std::int64_t Database::ttl(const std::string &key) {
  if (isExpired(key)) {
    return -2;
  }

  auto it = kv.find(key);
  if (it == kv.end()) {
    return -2;
  }

  if (!it->second.hasTTL) {
    return -1;
  }

  auto now = Clock::now();
  if (now >= it->second.expiresAt) {
    kv.erase(it);
    return -2;
  }

  auto remaining = std::chrono::duration_cast<std::chrono::seconds>(
                       it->second.expiresAt - now)
                       .count();
  return remaining;
}

ReturnState Database::incr(const std::string &key) {
  if (isExpired(key)) {
    return ReturnState::NotFound;
  }

  auto it = kv.find(key);
  if (it == kv.end()) {
    kv[key] = StoredValue{CompositeValue("1"), false, TimePoint{}};

    return ReturnState::Success;
  }

  if (!it->second.data.isString()) {
    return ReturnState::WrongType;
  }

  std::string &str = it->second.data.asString();

  try {
    long long number = std::stoll(str);
    number++;
    str = std::to_string(number);
  } catch (...) {
    return ReturnState::InvalidValue;
  }
  return ReturnState::Success;
}

ReturnState Database::decr(const std::string &key) {
  if (isExpired(key)) {
    return ReturnState::NotFound;
  }

  auto it = kv.find(key);
  if (it == kv.end()) {
    kv[key] = StoredValue{CompositeValue("-1"), false, TimePoint{}};

    return ReturnState::Success;
  }

  if (!it->second.data.isString()) {
    return ReturnState::WrongType;
  }

  std::string &str = it->second.data.asString();

  try {
    long long number = std::stoll(str);
    number--;
    str = std::to_string(number);
  } catch (...) {
    return ReturnState::InvalidValue;
  }
  return ReturnState::Success;
}

ReturnState Database::lpush(const std::string &key, const std::string &value) {
  if (isExpired(key)) {
  }

  auto it = kv.find(key);
  if (it == kv.end()) {
    std::vector<std::string> list;
    list.push_back(value);
    kv[key] = StoredValue{CompositeValue(list), false, TimePoint{}};
    return ReturnState::Success;
  }

  if (!it->second.data.isList()) {
    return ReturnState::WrongType;
  }

  auto &list = it->second.data.asList();
  list.insert(list.begin(), value);
  return ReturnState::Success;
}

ReturnState Database::rpush(const std::string &key, const std::string &value) {
  if (isExpired(key)) {
  }

  auto it = kv.find(key);
  if (it == kv.end()) {
    std::vector<std::string> list;
    list.push_back(value);

    kv[key] = StoredValue{CompositeValue(list), false, TimePoint{}};

    return ReturnState::Success;
  }

  if (!it->second.data.isList()) {
    return ReturnState::WrongType;
  }

  auto &list = it->second.data.asList();
  list.push_back(value);

  return ReturnState::Success;
}

ReturnState Database::lrange(const std::string &key, std::int64_t start,
                             std::int64_t end,
                             std::vector<std::string> &values) {
  values.clear();

  if (isExpired(key)) {
    return ReturnState::NotFound;
  }

  auto it = kv.find(key);

  if (it == kv.end()) {
    return ReturnState::NotFound;
  }
  if (!it->second.data.isList()) {
    return ReturnState::WrongType;
  }
  auto &list = it->second.data.asList();

  if (list.empty()) {
    return ReturnState::Success;
  }

  std::int64_t size = static_cast<std::int64_t>(list.size());

  if (start < 0) {
    start = size + start;
  }
  if (end < 0) {
    end = size + end;
  }
  if (start < 0) {
    start = 0;
  }
  if (end < 0 || start >= size || start > end) {
    return ReturnState::Success;
  }
  if (end >= size) {
    end = size - 1;
  }
  for (std::int64_t i = start; i <= end; i++) {
    values.push_back(list[i]);
  }

  return ReturnState::Success;
}

ReturnState Database::sadd(const std::string &key, const std::string &value) {
  if (isExpired(key)) {
  }

  auto it = kv.find(key);

  if (it == kv.end()) {
    std::unordered_set<std::string> set;
    set.insert(value);

    kv[key] = StoredValue{CompositeValue(set), false, TimePoint{}};

    return ReturnState::Success;
  }

  if (!it->second.data.isSet()) {
    return ReturnState::WrongType;
  }

  auto &set = it->second.data.asSet();
  set.insert(value);

  return ReturnState::Success;
}

ReturnState Database::srem(const std::string &key, const std::string &value) {
  if (isExpired(key)) {
    return ReturnState::NotFound;
  }

  auto it = kv.find(key);

  if (it == kv.end()) {
    return ReturnState::NotFound;
  }

  if (!it->second.data.isSet()) {
    return ReturnState::WrongType;
  }

  auto &set = it->second.data.asSet();
  set.erase(value);

  return ReturnState::Success;
}
