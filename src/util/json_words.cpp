//written by ananyaa sutaria

#include "util/json_words.hpp"
#include <fstream>
#include <sstream>
#include <unordered_map>

using namespace std;

static inline string to_lower_keep_apostrophe(const std::string& s) {
  string t; t.reserve(s.size());
  for (unsigned char c : s) {
    if (c == '\'') { t.push_back(c); }
    else if (c >= 'A' && c <= 'Z') { t.push_back(char(c - 'A' + 'a')); }
    else if ((c >= 'a' && c <= 'z') || c == ' ') { t.push_back(c); }
  }
  // trim spaces
  while (!t.empty() && t.front() == ' ') t.erase(t.begin());
  while (!t.empty() && t.back()  == ' ') t.pop_back();
  return t;
}

bool json_words::load_word_freq_json(const std::string& path, std::vector<WordRow>& out_rows) {
  ifstream in(path);
  if (!in) return false;
  ostringstream oss; oss << in.rdbuf();
  const string s = oss.str();

  unordered_map<string,int> tally;
  const string needle = "\"word\"";
  size_t pos = 0;
  while ((pos = s.find(needle, pos)) != string::npos) {
    size_t colon = s.find(':', pos + needle.size());
    if (colon == string::npos) break;
    size_t q1 = s.find('"', colon + 1);
    if (q1 == string::npos) break;
    size_t q2 = s.find('"', q1 + 1);
    if (q2 == string::npos) break;

    string raw = s.substr(q1 + 1, q2 - q1 - 1);
    string norm = to_lower_keep_apostrophe(raw);
    if (!norm.empty()) {
      tally[norm] += 1;
    }
    pos = q2 + 1;
  }

  out_rows.clear();
  out_rows.reserve(tally.size());
  for (auto& kv : tally) {
    out_rows.push_back(WordRow{kv.first, kv.second});
  }
  return true;
}
