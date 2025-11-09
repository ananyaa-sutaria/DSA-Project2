    //
    // Created by cath on 10/31/2025.
    //

    #include "index/trie_index.hpp"
    #include <iostream>
    #include <algorithm>
    #include <queue>
    #include <vector>
    #include <cstring>
    using namespace std;


//memory management

    // new TrieNode and tracking
    TrieNode* TrieIndex :: new_node()
    {
        TrieNode* node = new TrieNode();
        all_nodes.push_back(node);
        return node;
    }

    // cleans up dynamically allocated memory
    void TrieIndex :: clear()
    {
        // word_info is deleted before the node itself
        for (TrieNode* node : all_nodes)
        {
            delete node -> word_info;
            node->word_info = nullptr;
        }
        for (TrieNode* node : all_nodes)
        {
            delete node;
        }
        all_nodes.clear();
        root = nullptr;
    }

// building

    // Trie from vector of WordRow structures
    void TrieIndex::build_from_vocab(const vector<WordRow> &rows)
    {
        clear();
        root = new_node();
    //go through all words
        for (const WordRow& row : rows)
        {
            TrieNode* curr = root;
            const string& word = row.word;
            const int freq = row.freq;

            if (word.empty() || freq <=0)
            {
                continue;
            }

            for (size_t i = 0; i < word.size(); i++)
            {
                const char c = word[i];
                int index = c- char_offset;
                //boundary check for letters
                if (index <0 || index >= alphabet_size)
                {
                    break;
                }
                //create new node if missed
                if (curr->children[index] == nullptr)
                {
                    curr -> children[index] = new_node();
                }

                curr = curr -> children[index];
                // update maximum frequency
                curr-> max_suffix_freq = max(curr-> max_suffix_freq, freq);

                //update and store extra info at end of word
                if (i == word.size() - 1)
                {
                    int len = 0;
                    int offset = pool.add(word, len);
                    if (curr->word_info)
                    {  //update frequency
                        curr -> word_info->freq += freq;
                    }
                    else
                    {  //new word creation
                        curr -> word_info = new WordInfo{.string_pool_offset = offset, .len = len, .freq = freq};
                    }
                }
            }
        }
    }


// query and DFS
    // DFS for top k candidates starting from a given node.
    void TrieIndex::find_top_k_candidates(TrieNode *node, std::priority_queue<Candidate, std::vector<Candidate>, CandidateMinHeapComp>& top_k, int k) const
    {
        if (node == nullptr)
        {
            return;
        }
        // cast k to size_t for comparison with the heap's size()
        const size_t max_size = (size_t)k;

//optimizing/pruning
        // heap is full and the max frequency in this subtree is less than the frequency of worst candidate (top of the min-heap) -> prune this branch.
        if (top_k.size() == max_size && node->max_suffix_freq < top_k.top().freq)
        {
            return;
        }

        // check current node for a word
        if (node ->word_info != nullptr)
        {
            const WordInfo* info = node -> word_info;
            Candidate current_candidate = { .off = info -> string_pool_offset, .len = info-> len, .freq = info -> freq};

            if (top_k.size() < max_size)
            {
                // heap is not full -> add the candidate
                top_k.push(current_candidate);
            }
            else
            {
                // heap is full -> check if the current candidate is better than the worst (top) element
                if (current_candidate.freq > top_k.top().freq || (current_candidate.freq == top_k.top().freq &&  current_candidate.len < top_k.top().len))
                {
                    top_k.pop();
                    top_k.push(current_candidate);
                }
            }
        }
        // recurse on children
        for (int i = 0; i < alphabet_size; i++)
        {
            find_top_k_candidates(node -> children[i], top_k, k);
        }
    }

    // main query function: top k suggestions for a prefix.
    void TrieIndex :: query(const string& prefix, vector<Candidate>& output, int k ) const
    {
        output.clear();
        if (prefix.empty() || k <= 0 || root == nullptr)
        {
            return;
        }

        // traverse to the end
        TrieNode* curr = root;
        for (char c : prefix)
        {
            int index = c - char_offset;
            if (index < 0 || index >= alphabet_size || curr -> children[index] == nullptr)
            {
                // stop if prefix path not found
                return;
            }
            curr = curr -> children[index];
        }

        // use a min-heap to collect the top candidates
        priority_queue<Candidate, vector<Candidate>, CandidateMinHeapComp> pq;
        // DFS from the node where the prefix ends
        find_top_k_candidates(curr, pq, k);
        // transfer results from heap to the output vector
        while (!pq.empty())
        {
            output.push_back(pq.top());
            pq.pop();
        }

        // final sort: sort for best candidates first
        sort(output.begin(), output.end(), [](const Candidate& a, const Candidate& b)
        {
            // highest frequency first
            if (a.freq != b.freq)
            {
                return a.freq > b.freq;
            }
            //shortest length
            return a.len < b.len;
        });
    }

    //total memory usage of the Trie data structure
    size_t TrieIndex :: mem_bytes() const
    {
        size_t total_bytes = 0;
        size_t word_info_count = 0;

        total_bytes += all_nodes.size() * sizeof(TrieNode);

        for (const TrieNode* node : all_nodes)
        {
            if (node -> word_info)
            {
                word_info_count ++;
            }
        }

        total_bytes += word_info_count * sizeof(WordInfo);
        total_bytes += all_nodes.capacity() * sizeof(TrieNode*);
        return total_bytes;
    }
