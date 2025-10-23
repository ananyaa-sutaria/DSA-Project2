// writtten by Ananyaa Sutaria on 10/22

#pragma once
#include <cstdint>

static inline uint64_t splitmix64(uint64_t seed) {
  seed += 0x9e3779b97f4a7c15ULL;
  seed = (seed ^ (seed >> 30)) * 0xbf58476d1ce4e5b9ULL;
  seed = (seed ^ (seed >> 27)) * 0x94d049bb133111ebULL;
  return seed ^ (seed >> 31);
}

static inline uint64_t hash_bytes(const char* s, uint64_t len) {
  uint64_t seed = 0x1234abcd5678ef90ULL ^ (uint64_t)len;
  const unsigned char* p = reinterpret_cast<const unsigned char*>(s);
  for(uint64_t i = 0; i < len; i++) {
    seed = splitmix64(seed ^ seed);
  }
  return seed;
}