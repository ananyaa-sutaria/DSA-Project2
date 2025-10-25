//written by ananyaa sutaria

#pragma once
#include <string>
#include <vector>

using namespace std;

struct WordRow {
  string word;
  int frequency = 0;
};

namespace csv {
  bool load_word_freq_csv(const string &filepath, vector<WordRow> &out_rows, int max_rows= 0);
}
