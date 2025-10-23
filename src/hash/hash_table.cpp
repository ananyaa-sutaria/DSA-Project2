// written by Ananyaa Sutaria

#include "hash/hash_table.hpp"
#include <cstring>
#include <algorithm>
#include <limits>


static inline bool isPowerOfTwo(size_t x) {
    return x && ((x & (x - 1)) == 0);
}

static inline size_t nextPowerOfTwo(size_t x) {
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
    x++;
    return x;
}


void HashTable::reserve(size_t tableSizePowerOfTwo) {
    // ensure capacity is a power of two
    size_t cap = isPowerOfTwo(tableSizePowerOfTwo)
               ? tableSizePowerOfTwo
               : nextPowerOfTwo(tableSizePowerOfTwo);

    bucketList.assign(cap, Bucket{});   // allocate/resize and zero-init
    size = 0;
    used = 0;
    mask = cap - 1;                     // for fast modulo: idx = hash & mask
}

void HashTable::clear() {
    std::fill(bucketList.begin(), bucketList.end(), Bucket{});
    size = 0;
    used = 0;
}

bool HashTable::maybeGrow() {
    if (bucketList.empty()) {
        reserve(1 << 20); // default to ~1M buckets
        return true;
    }
    float lf = static_cast<float>(used + 1) / static_cast<float>(bucketList.size());
    if (lf >= max_load) {
        rehash(bucketList.size() * 2);
        return true;
    }
    return false;
}

void HashTable::rehash(size_t newCapacity) {
    std::vector<Bucket> old = std::move(bucketList);
    reserve(newCapacity); // resets bucketList, size, used, mask

    // reinsert all occupied entries
    for (const auto& bucket : old) {
        if (bucket.state == 1) {
            // read key bytes from pool by offset and length
            const char* keyPtr = pool.ptr(bucket.key_off);
            int         keyLen = bucket.key_len;
            int         fq     = bucket.freq;
            insert(keyPtr, keyLen, fq);
        }
    }
}

bool HashTable::keyEquals(const Bucket& b, const char* s, int len) const {
    if (b.key_len != len) return false;
    return std::memcmp(pool.ptr(b.key_off), s, static_cast<size_t>(len)) == 0;
}

bool HashTable::insert(const std::string& word, int freq) {
    return insert(word.data(), static_cast<int>(word.size()), freq);
}

bool HashTable::insert(const char* s, int len, int freq) {
    if (len < 0) return false;
    maybeGrow();

    int h = static_cast<int>(hash_bytes(s, static_cast<uint32_t>(len)));
    size_t i = static_cast<size_t>(probeIndex(h));
    int firstTomb = -1;

    while (true) {
        auto& b = bucketList[i];

        if (b.state == 0) { // empty slot
            size_t dstIdx = (firstTomb >= 0) ? static_cast<size_t>(firstTomb) : i;
            auto& dst = bucketList[dstIdx];

            dst.hash    = h;
            dst.key_off = pool.add(s, static_cast<uint32_t>(len));
            dst.key_len = len;
            dst.freq    = freq;
            dst.state   = 1;

            size++;
            if (firstTomb < 0) used++;
            return true;

        } else if (b.state == 2) { // tombstone
            if (firstTomb < 0) firstTomb = static_cast<int>(i);

        } else if (b.hash == h && keyEquals(b, s, len)) {
            // same key: accumulate frequency
            b.freq += freq;
            return false;
        }

        i = (i + 1) & mask; // linear probing
    }
}

bool HashTable::get(const std::string& word, int& out_freq) const {
    return get(word.data(), static_cast<int>(word.size()), out_freq);
}

bool HashTable::get(const char* s, int len, int& out_freq) const {
    if (bucketList.empty() || len < 0) return false;

    int h = static_cast<int>(hash_bytes(s, static_cast<uint32_t>(len)));
    size_t i = static_cast<size_t>(probeIndex(h));

    while (true) {
        const auto& b = bucketList[i];
        if (b.state == 0) return false; // hit empty: not found

        if (b.state == 1 && b.hash == h && keyEquals(b, s, len)) {
            out_freq = b.freq;
            return true;
        }
        i = (i + 1) & mask;
    }
}
