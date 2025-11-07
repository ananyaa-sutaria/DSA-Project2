//
// Created by cath on 10/31/2025.
//

#pragma once

#include <string>
#include <vector>
#include <queue>
#include <array>
#include <algorithm>
#include <cstdint>
#include <memory>
#include <iostream>
using namespace std;

#include "index/candidate.hpp"
#include "hash/string_pool.hpp"
#include "util/word_row.hpp"


struct CandidateMinHeapComp {
    bool operator() (const Candidate& a , const Candidate& b) const {
        // Return true if 'a' is considered "worse" (should bubble up, min-heap style) than 'b'.

        // 1. Primary sorting: Higher frequency is better (i.e., lower freq is worse).
        if (a.freq != b.freq) {
            // We want smaller frequency at the top, so if a.freq > b.freq, 'a' is "worse".
            return a.freq > b.freq;
        }

        // 2. Secondary sorting (Tie-breaker): Shorter length is better (i.e., longer length is worse).
        // If a.len > b.len, 'a' is worse, so we return true to put the longer word
        // (worse candidate) at the top of the min-heap.
        return a.len > b.len;
    }
};

static const int alphabet_size = 26;
static const int char_offset = 'a';

// info about word at end of node
struct WordInfo {
    int string_pool_offset = -1;
    int len = 0;
    int freq = 0;
};

struct TrieNode {
    TrieNode* children[alphabet_size] = {nullptr};
    WordInfo* word_info = nullptr;
    int max_suffix_freq = 0;
};

class TrieIndex{
    public:
    TrieIndex(StringPool& p) : pool(p) {
        root = new_node();
    }

    ~TrieIndex() {
        clear();
    }

    void build_from_vocab(const vector<WordRow>& rows);
    void query(const string& prefix, vector<Candidate>& output, int k) const;
    size_t mem_bytes() const;
    void clear();

    private:
    StringPool& pool;
    TrieNode* root= nullptr;

    vector<TrieNode*> all_nodes;
    TrieNode* new_node();
    void find_top_k_candidates(
        TrieNode* node,
        std::priority_queue<Candidate, std::vector<Candidate>, CandidateMinHeapComp>& top_k,
        int k
    ) const;
};

