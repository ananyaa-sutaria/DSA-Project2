#pragma once

#include <vector>
#include <string>
#include <cstdint>

class StringPool {
  public:
    uint32_t add(const std::string&s, uint32_t& out_len);
    uint32_t add(const char* s, uint32_t len);

    const char* ptr(uint32_t off) const {
        return data_.data() + off;
      }

    size_t size() const { return data_.size(); }
    void clear() { data_.clear(); data_.shrink_to_fit(); }

   private:
     std::vector<char> data_;
};