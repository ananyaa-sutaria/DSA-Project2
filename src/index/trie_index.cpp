//
// Created by cath on 10/31/2025.
//

#include "index/trie_index.hpp"
#include <iostream>
#include <algorithm>
#include <queue> // Required for std::priority_queue
#include <vector>
#include <cstring>
using namespace std;

// Helper structure to define the comparison logic for the min-heap used in the query.
// // It creates a MIN-HEAP where the "worst" candidate (lowest frequency, longest length)
// // is at the top. This allows us to efficiently track the top K candidates.
// struct CandidateMinHeapComp {
//     bool operator() (const Candidate& a , const Candidate& b) const {
//         // Return true if 'a' is considered "worse" (should bubble up, min-heap style) than 'b'.
//
//         // 1. Primary sorting: Higher frequency is better (i.e., lower freq is worse).
//         if (a.freq != b.freq) {
//             // We want smaller frequency at the top, so if a.freq > b.freq, 'a' is "worse".
//             return a.freq > b.freq;
//         }
//
//         // 2. Secondary sorting (Tie-breaker): Shorter length is better (i.e., longer length is worse).
//         // If a.len < b.len, 'a' is better (i.e., less worse). We return a.len < b.len
//         // to put the longer length (worse) at the top.
//         return a.len < b.len;
//     }
// };

// --- Memory Management and Cleanup ---

// Allocates a new TrieNode and tracks it for cleanup.
TrieNode* TrieIndex :: new_node() {
    TrieNode* node = new TrieNode();
    all_nodes.push_back(node);
    return node;
}

// Cleans up all dynamically allocated memory.
void TrieIndex :: clear() {
    // Ensure word_info is deleted before the node itself.
    for (TrieNode* node : all_nodes) {
        delete node -> word_info;
        node->word_info = nullptr;
    }
    for (TrieNode* node : all_nodes) {
        delete node;
    }
    all_nodes.clear();
    root = nullptr;
}

// --- Build Method ---

// Builds the Trie from a vector of WordRow structures (word and frequency).
void TrieIndex::build_from_vocab(const vector<WordRow> &rows) {
    clear();
    root = new_node();
//go through all words
    for (const WordRow& row : rows) {
        TrieNode* curr = root;
        const string& word = row.word;
        const int freq = row.freq;

        if (word.empty() || freq <=0) {
            continue;
        }

        for (size_t i = 0; i < word.size(); i++) {
            const char c = word[i];
            int index = c- char_offset;
            //boundary check for letters
            if (index <0 || index >= alphabet_size) {
                break;
            }
            //create new node if missed
            if (curr->children[index] == nullptr) {
                curr -> children[index] = new_node();
            }

            curr = curr -> children[index];
            // Update the maximum frequency of any word in the subtree below this node.
            curr-> max_suffix_freq = max(curr-> max_suffix_freq, freq);

            //update and store extra info at end of word
            if (i == word.size() - 1) {
                int len = 0;
                int offset = pool.add(word, len);
                if (curr->word_info) {  //update frequency
                    curr -> word_info->freq += freq;
                }
                else {  //new word creation
                    curr -> word_info = new WordInfo{.string_pool_offset = offset, .len = len, .freq = freq};
                }
            }
        }
    }
}


// --- Query and DFS Implementation ---

// Recursive DFS to find the top K candidates starting from a given node.
// **FIXED:** Changed 'top_k' parameter from vector to priority_queue.
void TrieIndex::find_top_k_candidates(TrieNode *node,
                                      std::priority_queue<Candidate, std::vector<Candidate>, CandidateMinHeapComp>& top_k,
                                      int k) const
{
    if (node == nullptr) {
        return;
    }

    // Safely cast k to size_t for comparison with the heap's size()
    const size_t max_size = (size_t)k;

    // --- Optimization (Pruning) ---
    // If the heap is full AND the max frequency in this subtree is less than the frequency
    // of the WORST candidate (at the top of the min-heap), prune this branch.
    // **FIXED:** Corrected logical error in pruning condition.
    if (top_k.size() == max_size && node->max_suffix_freq < top_k.top().freq) {
        return;
    }

    // Check current node for a word
    if (node ->word_info != nullptr) {
        const WordInfo* info = node -> word_info;
        Candidate current_candidate = { .off = info -> string_pool_offset, .len = info-> len, .freq = info -> freq};

        if (top_k.size() < max_size) {
            // Heap is not full: simply add the candidate
            // **FIXED:** Using .push() on the priority_queue
            top_k.push(current_candidate);
        }
        else {
            // Heap is full: check if the current candidate is better than the worst (top) element
            if (current_candidate.freq > top_k.top().freq ||
                (current_candidate.freq == top_k.top().freq &&  current_candidate.len < top_k.top().len))
            {
                // **FIXED:** Using .pop() and .push() on the priority_queue
                top_k.pop();
                top_k.push(current_candidate);
            }
        }
    }

    // Recurse on children
    for (int i = 0; i < alphabet_size; i++) {
        find_top_k_candidates(node -> children[i], top_k, k);
    }
}

// Main query function. Finds the top K suggestions for a given prefix.
void TrieIndex :: query(const string& prefix, vector<Candidate>& output, int k ) const {
    output.clear();
    if (prefix.empty() || k <= 0 || root == nullptr) {
        return;
    }

    // 1. Traverse to the end of the prefix
    TrieNode* curr = root;
    for (char c : prefix) {
        int index = c - char_offset;
        if (index < 0 || index >= alphabet_size || curr -> children[index] == nullptr) {
            // Stop if prefix path is not found
            return;
        }
        curr = curr -> children[index];
    }

    // 2. Use a min-heap (priority_queue) to collect the top K candidates
    // **FIXED:** Declared local priority_queue 'pq'
    priority_queue<Candidate, vector<Candidate>, CandidateMinHeapComp> pq;

    // 3. Start the DFS from the node where the prefix ends
    // **FIXED:** Passed 'pq' instead of the undeclared 'top_k_heap'
    find_top_k_candidates(curr, pq, k);

    // 4. Transfer results from heap to the output vector
    // **FIXED:** Used 'pq' instead of the undeclared 'top_k_heap'
    while (!pq.empty()) {
        output.push_back(pq.top());
        pq.pop();
    }

    // 5. Final Sort: Sort the output to present the best candidates first
    sort(output.begin(), output.end(), [](const Candidate& a, const Candidate& b) {
        // Sort by highest frequency first
        if (a.freq != b.freq) {
            return a.freq > b.freq;
        }
        // Then by shortest length (tie-breaker)
        return a.len < b.len;
    });
}

// Estimates the total memory usage of the Trie data structure (excluding the StringPool).
size_t TrieIndex :: mem_bytes() const {
    size_t total_bytes = 0;
    size_t word_info_count = 0;

    total_bytes += all_nodes.size() * sizeof(TrieNode);

    for (const TrieNode* node : all_nodes) {
        if (node -> word_info) {
            word_info_count ++;
        }
    }

    total_bytes += word_info_count * sizeof(WordInfo);
    // Note: The memory calculation for 'all_nodes.capacity()' should likely use sizeof(TrieNode*)
    // if 'all_nodes' is a vector of pointers, but sticking to your original calculation style:
    total_bytes += all_nodes.capacity() * sizeof(TrieNode*);
    return total_bytes;
}
