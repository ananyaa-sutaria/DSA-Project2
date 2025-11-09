//
// Created by Anyaa Sutaria on 10/26/25.
//

#include "util/csv.hpp"
#include <fstream>
#include <sstream>

namespace csv {

    static inline bool parse_line(const std::string& line, WordRow& row) {
        if (line.empty()) return false;
        size_t c = line.find(',');
        if (c == std::string::npos) return false;
        std::string w = line.substr(0, c);
        std::string f = line.substr(c + 1);
        if (w.empty() || f.empty()) return false;
        // trim spaces
        auto trim = [](std::string& s){
            while (!s.empty() && (s.back()=='\r' || s.back()==' ')) s.pop_back();
            size_t i=0; while (i<s.size() && s[i]==' ') ++i;
            if (i>0) s.erase(0,i);
        };
        trim(w); trim(f);
        row.word = w;
        row.freq = (uint32_t)std::stoul(f);
        return true;
    }

    bool load_word_freq_csv(const std::string& path, std::vector<WordRow>& out_rows, size_t max_rows_hint) {
        std::ifstream in(path);
        if (!in) return false;
        out_rows.clear(); if (max_rows_hint) out_rows.reserve(max_rows_hint);
        std::string line;
        bool first = true;
        while (std::getline(in, line)) {
            if (first) {
                // header check
                if (line.find("word") != std::string::npos && line.find("freq") != std::string::npos) {
                    first = false;
                    continue;
                }
                first = false;
            }
            WordRow r;
            if (parse_line(line, r)) out_rows.push_back(std::move(r));
        }
        return true;
    }

} // namespace csv
