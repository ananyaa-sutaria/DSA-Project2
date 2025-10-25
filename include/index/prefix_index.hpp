//written by ananyaa sutaria

#include <cstdint>
#include <vector>
#include <string>
#include "hash/hash_table.hpp"
#include "hash/splitmix64.hpp"
#include "hash/string_pool.hpp"

using namespace std;

struct Candidate {
  int off = 0;
  int len = 0;
  int freq = 0;
};

struct PrefixList {
  std::vector<Candidate> items;
};

struct PrefixBucket {
  int hash = 0;
  int key_off = 0;
  int key_len = 0;
  int list_id = -1;
  int state = 0;
};

class PrefixIndex {
  public:
    PrefixIndex(StringPool& p, int len, int nums) : pool(p), L(len), K(nums) { reserve(1 << 18); }

    void reserve(int capacity2);
    void clear();

    void build_from_vocab(const HashTable& vocab);
    void query(const string& prefix, vector<Candidate>& output) const;

    size_t mem_bytes() const;

  private:
    StringPool& pool;
    vector<PrefixBucket> table;
    vector<PrefixList> lists;
    int size = 0;
    int used = 0;
    int mask = 0;
    int L = 6;
    int K = 10;

    bool maybe_grow();
    void rehash(int new_capacity);
    int upsert_list(const char* pre,int plen);
    void insert_candidate(int list_id, Candidate c);

    inline int probe_index(int h) const { return int(h) & int(mask);}
    bool key_equals(const PrefixBucket& b, const char*  s, int len) const;
};