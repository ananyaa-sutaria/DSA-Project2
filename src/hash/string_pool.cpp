// written by ananyaa sutaria

#include "hash/string_pool.hpp"

using namespace std;

int StringPool::add(const string& s, int& out_len) {
  int off = (int)data.size();
  data.insert(data.end(), s.begin(), s.end());
  out_len = (int)s.size();
  return off;
}

int StringPool::add(const char* s, int len) {
  int off = (int)data.size();
  data.insert(data.end(), s, s + len);
  return off;
}