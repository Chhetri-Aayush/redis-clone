#include "./database.h"
#include <fstream>

namespace {
void writeString(std::ofstream &out, const std::string &s) {
  uint32_t len = static_cast<uint32_t>(s.size());
  out.write(reinterpret_cast<const char *>(&len), sizeof(len));
  if (len)
    out.write(s.data(), len);
}

std::string readString(std::ifstream &in) {
  uint32_t len = 0;
  in.read(reinterpret_cast<char *>(&len), sizeof(len));
  std::string s(len, '\0');
  if (len)
    in.read(&s[0], len);
  return s;
}
} // namespace

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
  // kv[key] = StoredValue{CompositeValue(value), false, TimePoint{}};
  kv.insert_or_assign(key,
                      StoredValue{CompositeValue(value), false, TimePoint{}});
  return ReturnState::Success;
}

ReturnState Database::set(const std::string &key, const std::string &value,
                          std::int64_t ttlSeconds) {
  if (ttlSeconds <= 0) {
    return ReturnState::InvalidValue;
  }

  kv.insert_or_assign(
      key, StoredValue{CompositeValue(value), true,
                       Clock::now() + std::chrono::seconds(ttlSeconds)});

  // kv[key] = StoredValue{CompositeValue(value), true,
  //                       Clock::now() + std::chrono::seconds(ttlSeconds)};
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
    // kv[key] = StoredValue{CompositeValue("1"), false, TimePoint{}};
    kv.insert_or_assign(key,
                        StoredValue{CompositeValue("1"), false, TimePoint{}});
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
    // kv[key] = StoredValue{CompositeValue("-1"), false, TimePoint{}};
    kv.insert_or_assign(key,
                        StoredValue{CompositeValue("-1"), false, TimePoint{}});
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
    kv.insert_or_assign(key,
                        StoredValue{CompositeValue(list), false, TimePoint{}});
    // kv[key] = StoredValue{CompositeValue(list), false, TimePoint{}};
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

    kv.insert_or_assign(key,
                        StoredValue{CompositeValue(list), false, TimePoint{}});
    // kv[key] = StoredValue{CompositeValue(list), false, TimePoint{}};

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

    kv.insert_or_assign(key,
                        StoredValue{CompositeValue(set), false, TimePoint{}});
    // kv[key] = StoredValue{CompositeValue(set), false, TimePoint{}};
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

bool Database::saveToDisk(const std::string &path) {
  std::ofstream out(path, std::ios::binary | std::ios::trunc);
  if (!out)
    return false;

  uint32_t written = 0;
  auto countPos = out.tellp();
  out.write(reinterpret_cast<char *>(&written), sizeof(written));

  auto now = Clock::now();

  for (auto &[key, stored] : kv) {
    if (stored.hasTTL && now >= stored.expiresAt) {
      continue;
    }

    writeString(out, key);

    uint8_t typeTag = static_cast<uint8_t>(stored.data.typeIndex());
    out.write(reinterpret_cast<char *>(&typeTag), sizeof(typeTag));

    int64_t remaining = -1;
    if (stored.hasTTL) {
      remaining = std::chrono::duration_cast<std::chrono::seconds>(
                      stored.expiresAt - now)
                      .count();
    }
    out.write(reinterpret_cast<char *>(&remaining), sizeof(remaining));

    switch (stored.data.typeIndex()) {
    case 0: {
      writeString(out, stored.data.asString());
      break;
    }
    case 1: {
      auto &list = stored.data.asList();
      uint32_t n = static_cast<uint32_t>(list.size());
      out.write(reinterpret_cast<char *>(&n), sizeof(n));
      for (auto &item : list)
        writeString(out, item);
      break;
    }
    case 2: {
      auto &set = stored.data.asSet();
      uint32_t n = static_cast<uint32_t>(set.size());
      out.write(reinterpret_cast<char *>(&n), sizeof(n));
      for (auto &item : set)
        writeString(out, item);
      break;
    }
    case 3: {
      auto &map = stored.data.asMap();
      uint32_t n = static_cast<uint32_t>(map.size());
      out.write(reinterpret_cast<char *>(&n), sizeof(n));
      for (auto &[k, v] : map) {
        writeString(out, k);
        writeString(out, v);
      }
      break;
    }
    }

    written++;
  }

  auto endPos = out.tellp();
  out.seekp(countPos);
  out.write(reinterpret_cast<char *>(&written), sizeof(written));
  out.seekp(endPos);

  return static_cast<bool>(out);
}

bool Database::loadFromDisk(const std::string &path) {
  std::ifstream in(path, std::ios::binary);
  if (!in)
    return false;

  uint32_t count = 0;
  in.read(reinterpret_cast<char *>(&count), sizeof(count));
  if (!in)
    return false;

  for (uint32_t i = 0; i < count && in; i++) {
    std::string key = readString(in);

    uint8_t typeTag = 0;
    in.read(reinterpret_cast<char *>(&typeTag), sizeof(typeTag));

    int64_t remaining = -1;
    in.read(reinterpret_cast<char *>(&remaining), sizeof(remaining));

    switch (typeTag) {
    case 0: {
      std::string val = readString(in);
      kv.insert_or_assign(key,
                          StoredValue{CompositeValue(val), false, TimePoint{}});
      break;
    }
    case 1: {
      uint32_t n = 0;
      in.read(reinterpret_cast<char *>(&n), sizeof(n));
      std::vector<std::string> list;
      list.reserve(n);
      for (uint32_t j = 0; j < n; j++)
        list.push_back(readString(in));
      kv.insert_or_assign(
          key, StoredValue{CompositeValue(list), false, TimePoint{}});
      break;
    }
    case 2: {
      uint32_t n = 0;
      in.read(reinterpret_cast<char *>(&n), sizeof(n));
      std::unordered_set<std::string> set;
      for (uint32_t j = 0; j < n; j++)
        set.insert(readString(in));
      kv.insert_or_assign(key,
                          StoredValue{CompositeValue(set), false, TimePoint{}});
      break;
    }
    case 3: {
      uint32_t n = 0;
      in.read(reinterpret_cast<char *>(&n), sizeof(n));
      std::unordered_map<std::string, std::string> map;
      for (uint32_t j = 0; j < n; j++) {
        std::string k = readString(in);
        std::string v = readString(in);
        map[k] = v;
      }
      kv.insert_or_assign(key,
                          StoredValue{CompositeValue(map), false, TimePoint{}});
      break;
    }
    default:
      return false;
    }

    // if (remaining >= 0) {
    //   kv[key].hasTTL = true;
    //   kv[key].expiresAt = Clock::now() + std::chrono::seconds(remaining);
    // }
    if (remaining >= 0) {
      auto it = kv.find(key);
      if (it != kv.end()) {
        it->second.hasTTL = true;
        it->second.expiresAt = Clock::now() + std::chrono::seconds(remaining);
      }
    }
  }

  return true;
}
