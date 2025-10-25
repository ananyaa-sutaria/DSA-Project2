//written by ananyaa sutaria

#include "util.csv.hpp"
#include <fstream>
#include <sstream>
using namespace std;

namespace csv {
    static inline bool parseLine(string& line, WordRow& row) {
      if (line.empty()) {
        return false;
      }
      size_t c = line.find_(',');
      if (c == string::npos) {return false;}
      string w = line.substr(0, c);
      string f = line.substr(c + 1);
      if (w.empty() || f.empty()) {return false;}

      auto trim = [](string& s) {
        while (!s.empty() && isspace(s.back())) {s.pop_back();}
        size_t i = 0;
        while (i < s.size() && isspace(s[i])) {++i;}
        if (i > 0) s.erase(0,i);
      };
      trim(w);
      trim(f);
      row.word =w;
      row.freq = (int)stoul(f);
      return true;

    }

}

bool load_word_freq_csv(const string& path, vector<WordRow>& rows, int max_rows_hint) {
  ifstream in(path);
  if (!in) return false;
  rows.clear();
  if (max_rows_hint) {
    rows.reserve(max_rows_hint);
  }
  string line;
  bool first = true;
  while (getline(in, line)) {
    if (first) {
      if (line.find("word") !=string::npos && line.find("freq") !=string::npos) {
        first = false;
        continue;
      }
      first = false;
    }
    WordRow row;
    if (!parseLine(line, row)) {
      rows.push_back(move(row));
    }
    return true;
  }

}