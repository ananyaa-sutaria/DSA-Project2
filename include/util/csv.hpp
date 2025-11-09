//
// Created by Anyaa Sutaria on 10/26/25.
//

#pragma once
#include <string>
#include <vector>
#include <cstdint>
# include "util/word_row.hpp"


namespace csv {
    // Load "word,freq" with header optional. Skips blank lines.
    bool load_word_freq_csv(const std::string& path, std::vector<WordRow>& out_rows, size_t max_rows_hint = 0);
}
