#ifndef DATABASE_H
#define DATABASE_H

#include <chrono>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

enum class ReturnState { Success, NotFound, WrongType, InvalidValue };

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

  std::size_t typeIndex() const { return value.index(); }

  std::string &asString();
  std::vector<std::string> &asList();
  std::unordered_set<std::string> &asSet();
  std::unordered_map<std::string, std::string> &asMap();
};

class Database {
private:
  using Clock = std::chrono::steady_clock;
  using TimePoint = Clock::time_point;

  struct StoredValue {
    CompositeValue data;
    bool hasTTL = false;
    TimePoint expiresAt;
  };

  std::unordered_map<std::string, StoredValue> kv;
  bool isExpired(const std::string &key);

public:
  ReturnState del(const std::string &key);
  ReturnState exist(const std::string &key);
  ReturnState expire(const std::string &key, std::int64_t seconds);
  std::int64_t ttl(const std::string &key);
  // string
  ReturnState set(const std::string &key, const std::string &value);
  ReturnState set(const std::string &key, const std::string &value,
                  std::int64_t ttlSeconds);
  ReturnState get(const std::string &key, std::string &value);
  ReturnState incr(const std::string &key);
  ReturnState decr(const std::string &key);

  // list
  ReturnState lpush(const std::string &key, const std::string &value);
  ReturnState rpush(const std::string &key, const std::string &value);
  ReturnState lrange(const std::string &key, std::int64_t start,
                     std::int64_t end, std::vector<std::string> &values);
  // set
  ReturnState sadd(const std::string &key, const std::string &value);
  ReturnState srem(const std::string &key, const std::string &value);
  // persistence
  bool saveToDisk(const std::string &path);
  bool loadFromDisk(const std::string &path);
};

#endif
