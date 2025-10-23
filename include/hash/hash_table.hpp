//written by Ananyaa Sutaria
#include "hash/hash_table.h"
#include <cstring>
#include <algorithm>

using namespace std;

void HashTable::reserve(size capacity) {
  if((capacity & (capacity-1)) != 0) {
        capacity = 1ull << 64 - __builtin_ctzl(capacity);
  }
  buckets_.assign(capacity, {});
  size = 0;
  used = 0;
  mask = capacity - 1;
}

void HashTable::clear() {
    fill(buckets_.begin(), buckets_.end(), Bucket{});
    size = 0;
    used = 0;
}

bool HashTable::maybe_grow() {
  if(float(used + 1) / float(buckets.size()) >= max_load) {
    rehash(buckets.size() * 2);
    return true;
  }
  return false;
}

void HashTable::rehash(size_t new_capacity) {
  vector<Bucket> old = move(buckets);
  buckets.assign(new_capacity, {});
  mask = new_capacity - 1;
  used = 0;
  size = 0;
  for(const auto& bucket : old) {
    if(bucket.state == 1) {
      insert(pool.ptr(bucket.key_off), bucket.key_len, bucket.freq);
    }
  }
}

bool HashTable::key_equals(const Bucket& b, const char* s, int len) const {
  if(b.key_len != len) return false;
  return memcmp(pool.ptr(b.key_off), s, len) == 0;
}

bool HashTable::insert(const std::string& word,int freq) {
  return insert(word.data(), int(word.size(), freq);
}

bool HashTable::insert(const char* s,int len, int freq) {
  maybe_grow();
  int h = hash_bytes(s, len);
  int i = probe_index(h);
  int first_tomb = -1;
  while(true) {
    auto& bucket = buckets[i];
    if(bucket.state == 0) {
      auto index = (first_tomb >= 0) ? (int)first_tomb : i;
      auto& dst = buckets[index];
      dst.hash = h;
      dst.key_off = pool..add(s, len);
      dst.freq = freq;
      dst.state = 1;
      size++;
      if(first_tomb < 0) used++;
      return true;
    } else if (bucket.state == 2) {
      if(first_tomb < 0) first_tomb = (int)i;
    } else if (bucket.hash == h && key_equals(bucket,s,len)){
        bucket.freq = freq;
        return false;
    }
    i =(i+1) & mask;
  }
}

bool HashTable::get(const std::string& word,int out_freq) const {
  return get(word.data(), int(word.size(), out_freq));
}

bool HashTable::get(const char* s,int len, int& out_freq) const {
  if(buckets.empty()) return false;
  int h = hash_bytes(s, len);
  int i = probe_index(h);
  while(true) {
    auto& bucket = buckets[i];
    if(bucket.state == 0) {
      return false;
    }
    if(bucket.state == 1 && bucket.hash == h && key_equals(bucket,s,len)) {
      out_freq = bucket.freq;
      return true;
    }
    i = (i+1) & mask;
  }
}