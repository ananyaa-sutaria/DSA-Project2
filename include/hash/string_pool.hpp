#pragma once

#include <vector>
#include <string>
#include <cstdint>

class StringPool {
  public:
    int add(const std::string&s, int& out_len);
    int add(const char* s, int len);

    const char* ptr(int off) const {
        return data.data() + off;
      }

    size_t size() const { return data.size(); }
    void clear() { data.clear(); data.shrink_to_fit(); }

   private:
     std::vector<char> data;
};