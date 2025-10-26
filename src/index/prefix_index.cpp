// written by Ananyaa Sutaria

#include "index/prefix_index.hpp"
#include <algorithm>
#include <cstring>

using namespace std;

static inline size_t nextPow2(size_t x) {
  if (x < 2) return 2;
  x--;
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
#if ULONG_MAX > 0xFFFFFFFFUL
  x |= x >> 32;
#endif
  return x + 1;
}

void PrefixIndex::reserve(int capacity) {
  size_t cap = static_cast<size_t>(capacity);
  size_t newCap = (cap & (cap - 1)) ? nextPow2(cap) : cap;

  table.assign(newCap, PrefixBucket{});
  lists.clear();
  size = 0;
  used = 0;
  mask = static_cast<int>(newCap - 1);
}


bool PrefixIndex::maybe_grow() {
  if (table.empty()) { reserve(1 << 18); return true; }
  float lf = float(used + 1) / float(table.size());
  if (lf >= 0.7f) { rehash(static_cast<int>(table.size() * 2)); return true; }
  return false;
}


void PrefixIndex::rehash(int new_capacity) {
  vector<PrefixBucket> old = move(table);
  table.assign(new_capacity, {});
  mask = new_capacity - 1;
  used = 0;
  size = 0;

  for(auto& bucket : old) {
    if(bucket.state == 1) {
      int h = bucket.hash;
      int i = int(h) & int(mask);
      while(true) {
        auto& new_bucket = table[i];
        if(new_bucket.state == 0) {
          new_bucket = bucket;
          new_bucket.state = 1;
          used++;
          size++;
          break;
        }
        i = (i + 1) & int(mask);
      }
    }
  }
}

bool PrefixIndex::key_equals(const PrefixBucket & b, const char* s, int len) const {
  if (b.key_len != len) return false;
  return memcmp(pool.ptr(b.key_off), s, len) == 0;
}

int PrefixIndex::upsert_list(const char* pre, int plen) {
  if (plen > (int) plen)
    plen = L;
  int h = hash_bytes(pre, plen);
  maybe_grow();
  int i = int(h) & int(mask);
  int first_tomb = -1;
  while(true) {
    auto& bucket = table[i];
    if (bucket.state == 0) {
      int index = (first_tomb >= 0) ? first_tomb : i;
      auto& destination = table[index];

      destination.hash    = h;
      destination.key_len = plen;
      destination.key_off = pool.add(pre, plen);
      destination.state   = 1;

      // >>> set list_id BEFORE returning <<<
      destination.list_id = static_cast<int>(lists.size());
      lists.push_back(PrefixList{});
      lists.back().items.reserve(K);

      size++;
      if (first_tomb < 0) used++;   // <— note: new occupied bucket
      return destination.list_id;


  } else if (bucket.state == 2) {
      if(first_tomb < 0)
        first_tomb = i;
    } else if (bucket.hash ==h && key_equals(bucket, pre, plen)) {
        return bucket.list_id;
    }
    i = (i + 1) & int(mask);
  }

}

void PrefixIndex::insert_candidate(int list_id,Candidate candidate) {
  auto& lst = lists[list_id].items;

  if ((int)lst.size() < K) {
    lst.push_back(candidate);
    std::push_heap(lst.begin(), lst.end(),
        [](const Candidate& a, const Candidate& b){ return a.freq > b.freq; }); // min-heap by freq
  } else {
    std::make_heap(lst.begin(), lst.end(),
        [](const Candidate& a, const Candidate& b){ return a.freq > b.freq; });
    if (candidate.freq > lst.front().freq) {
      std::pop_heap(lst.begin(), lst.end(),
          [](const Candidate& a, const Candidate& b){ return a.freq > b.freq; });
      lst.back() = candidate;
      std::push_heap(lst.begin(), lst.end(),
          [](const Candidate& a, const Candidate& b){ return a.freq > b.freq; });
    }
  }
}


void PrefixIndex::build_from_vocab(const HashTable& vocab) {
  clear(); reserve(1 << 18);
  const auto& buckets = vocab.getBuckets();
  for (const auto& b : buckets) {
    if (b.state != 1) continue;
    const char* w = vocab.getPool().ptr(b.key_off);
    int len  = b.key_len;
    int freq = b.freq;
    for (int plen = 1; plen <= (int)L && plen <= len; ++plen) {
      int id = upsert_list(w, plen);
      insert_candidate(id, Candidate{ b.key_off, len, freq });
    }
  }
  for (auto& pl : lists) {
    auto& v = pl.items;
    sort(v.begin(), v.end(), [](const Candidate& a, const Candidate& b){
        if (a.freq != b.freq) return a.freq > b.freq;
        return a.len < b.len;
    });
    if ((int)v.size() > K) v.resize(K);
  }
}

void PrefixIndex::query(const std::string& prefix, std::vector<Candidate>& out) const {
  out.clear();
  int plen = (int)prefix.size();
  if (plen == 0) return;
  if (plen > (int)L) plen = L;
  int h = hash_bytes(prefix.data(), plen);
  int i = int(h) & int(mask);
  while (true) {
    const auto& b = table[i];
    if (b.state == 0) return;
    if (b.state == 1 && b.hash == h &&
        key_equals(b, prefix.data(), plen)) {
      const auto& lst = lists[b.list_id].items;
      out = lst;
      return;
        }
    i = (i + 1) & mask;
  }
}

size_t PrefixIndex::mem_bytes() const {
  size_t table_bytes = table.size() * sizeof(PrefixBucket);
  size_t lists_bytes = 0;
  for (auto& pl : lists) lists_bytes += pl.items.capacity() * sizeof(Candidate);
  return table_bytes + lists_bytes;
}

void PrefixIndex::clear() {
  std::fill(table.begin(), table.end(), PrefixBucket{});
  lists.clear();
  size = 0;
  used = 0;
}
