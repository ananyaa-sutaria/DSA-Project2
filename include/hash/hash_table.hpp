#pragma once
#include <vector>
#include <string>
#include "hash/splitmix64.hpp"
#include "hash/string_pool.hpp"

struct Bucket {
  int hash = 0;
  int key_off = 0;
  int key_len = 0;
  int freq = 0;
  int state = 0; // 0=empty, 1=occupied, 2=tombstone
};

class HashTable {
public:
  explicit HashTable(StringPool& p) : pool(p) { reserve(1 << 20); }

  void reserve(size_t tableSizePowerOfTwo);
  void clear();

  bool insert(const std::string& word, int freq);
  bool insert(const char* s, int len, int freq);

  bool get(const std::string& word, int& out_freq) const;
  bool get(const char* s, int len, int& out_freq) const;

  // Renamed these so they don’t conflict with variable names
  size_t getSize() const { return size; }
  size_t getCapacity() const { return bucketList.size(); }
  float getLoadFactor() const { return static_cast<float>(size) / static_cast<float>(bucketList.size()); }

  const StringPool& getPool() const { return pool; }
  const std::vector<Bucket>& getBuckets() const { return bucketList; }

private:
  StringPool& pool;
  std::vector<Bucket> bucketList;  // renamed from buckets to avoid collision
  size_t size = 0;
  size_t used = 0;
  size_t mask = 0;
  float max_load = 0.7f;

  void rehash(size_t newCapacity);
  bool maybeGrow();

  inline int probeIndex(int h) const { return h & static_cast<int>(mask); }
  bool keyEquals(const Bucket& b, const char* s, int len) const;
};
