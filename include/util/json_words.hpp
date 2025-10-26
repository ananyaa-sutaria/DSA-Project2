//written by ananyaa sutaria

#pragma once
#include <string>
#include <vector>
# include "util/word_row.hpp"


using namespace std;

namespace json_words {
  bool load_word_freq_json(const string &file_name, vector<WordRow> &words);
}

