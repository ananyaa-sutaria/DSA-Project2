#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <algorithm>
#include <limits>

#include "hash/string_pool.hpp"
#include "hash/hash_table.hpp"
#include "index/prefix_index.hpp"
#include "index/trie_index.hpp"
#include "index/candidate.hpp"
#include "util/json_words.hpp"
#include "util/csv.hpp"
#include "util/word_row.hpp"

using namespace std;

const int INDEX_HASH = 1;
const int INDEX_TRIE = 2;

const int K = 10;
const int L = 6;

StringPool pool;
HashTable vocab(pool);
std::vector<WordRow> rows;
PrefixIndex pidx(pool, L, K);
TrieIndex trie_idx(pool);

int current_index = INDEX_HASH; // default hashtable

// timers and status
long hash_build_time_us = 0;
long trie_build_time_us = 0;
bool is_data_loaded = false;
bool is_hash_built = false;
bool is_trie_built = false;

static inline int now_us()
{
  using namespace std::chrono;
  return duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();
}
// menu actions
void load_data_action(const std::string& path) {
    rows.clear();
    vocab.clear();
    pool.clear();
    pidx.clear();
    trie_idx.clear();
    is_hash_built = false;
    is_trie_built = false;

    auto ends_with = [](const std::string& s, const std::string& suffix){
        return s.size() >= suffix.size() && s.compare(s.size()-suffix.size(), suffix.size(), suffix) == 0;
    };

    bool ok = false;
    int t0 = now_us();
    if (ends_with(path, ".json")) {
        ok = json_words::load_word_freq_json(path, rows);
    }
    else {
        ok = csv::load_word_freq_csv(path, rows);
    }
    int t1 = now_us();

    if (!ok) {
        cerr << "FATAL ERROR: Failed to load dataset from: " << path << "\n";
        is_data_loaded = false;
        return;
    }

    // build HashTable (vocab) for exact lookups
    int t2 = now_us();
    vocab.reserve(rows.size());
    for (const auto& row : rows) {
        vocab.insert(row.word, row.freq);
    }
    int t3 = now_us();

    is_data_loaded = true;
    cout << "\n--- Dataset Loaded ---\n";
    cout << "Loaded " << rows.size() << " words in " << (t1-t0) / 1000.0 << " ms (I/O).\n";
    cout << "Built Vocabulary (HashTable) in " << (t3-t2) / 1000.0 << " ms.\n";
    cout << "Vocabulary size: " << vocab.getSize() << "\n";
}

void build_structure_action() {
    if (!is_data_loaded) {
        cout << "Please load a dataset first [1].\n";
        return;
    }

    cout << "Select structure to build:\n";
    cout << "  [1] Hash-based Index (PrefixIndex)\n";
    cout << "  [2] Trie-based Index (TrieIndex)\n> ";

    int sub_ch;
    if (!(std::cin >> sub_ch)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        cout << "Invalid input. Please enter 1 or 2.\n";
        return;
    }
    // spacing
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');


    if (sub_ch == INDEX_HASH) {
        // build Hash-based Index (PrefixIndex)
        pidx.clear();
        int t0 = now_us();
        // PrefixIndex builds from the HashTable (vocab)
        pidx.build_from_vocab(vocab);
        hash_build_time_us = now_us() - t0;
        is_hash_built = true;
        cout << "\n--- Hash-based Index Built ---\n";
        cout << "Build time: " << hash_build_time_us / 1000.0 << " ms\n";
    } else if (sub_ch == INDEX_TRIE) {
        // build Trie-based Index (TrieIndex)
        trie_idx.clear();
        int t0 = now_us();
        // TrieIndex builds from original vector of WordRows (rows)
        trie_idx.build_from_vocab(rows);
        trie_build_time_us = now_us() - t0;
        is_trie_built = true;
        cout << "\n--- Trie-based Index Built ---\n";
        cout << "Build time: " << trie_build_time_us / 1000.0 << " ms\n";
    } else {
        cout << "Invalid selection.\n";
    }
}

void query_prefix_action() {
    if ((current_index == INDEX_HASH && !is_hash_built) || (current_index == INDEX_TRIE && !is_trie_built)) {
        cout << "Please build the selected index first [2].\n";
        return;
    }

    string pre;
    cout << "\nEnter prefix to query: ";
    if (!(cin >> pre)) {
        cin.clear();
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        cout << "Invalid input.\n";
        return;
    }
    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    vector<Candidate> out;
    int a = now_us();

    if (current_index == INDEX_HASH) {
        // Hash Index Query
        pidx.query(pre, out);
        cout << "Using: Hash-based Index (L=" << L << ", K=" << K << ")\n";
    } else {
        // Trie Index Query
        trie_idx.query(pre, out, K);
        cout << "Using: Trie-based Index (K=" << K << ")\n";
    }

    int b = now_us();
    display_results(out, pre, (b - a));
}

void exact_lookup_action() {
    if (!is_data_loaded) {
        cout << "Please load a dataset first [1].\n";
        return;
    }
    string w;
    cout << "\nEnter word for exact lookup: ";
    if (!(cin >> w)) {
        cin.clear();
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        cout << "Invalid input.\n";
        return;
    }
    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    int f = 0;
    int a = now_us();
    // exact lookup uses HashTable (vocab)
    bool ok = vocab.get(w, f);
    int b = now_us();

    cout << "\n--- Exact Lookup ---\n";
    if (ok)
        cout << "Word found! Frequency = " << f << "\n";
    else
        cout << "Word not found.\n";
    cout << "Time: " << (b - a) << " us\n";
}

void display_stats_action() {
    cout << "\n--- Index Statistics ---\n";

    // Hash Index Stats
    cout << "\n[1] Hash-based Index (PrefixIndex):\n";
    if (is_hash_built) {
        cout << "  Build Status: BUILT\n";
        cout << "  Build Time: " << hash_build_time_us / 1000.0 << " ms\n";
        cout << "  Memory Usage: " << pidx.mem_bytes() / (1024.0 * 1024.0) << " mB\n";
        cout << "  Parameters: L=" << L << ", K=" << K << "\n";
    } else {
        cout << "  Build Status: NOT BUILT\n";
    }

    // Trie Index Stats
    cout << "\n[2] Trie-based Index (TrieIndex):\n";
    if (is_trie_built) {
        cout << "  Build Status: BUILT\n";
        cout << "  Build Time: " << trie_build_time_us / 1000.0 << " ms\n";
        cout << "  Memory Usage: " << trie_idx.mem_bytes() / (1024.0 * 1024.0) << " mB\n";
    } else {
        cout << "  Build Status: NOT BUILT\n";
    }

    // global stats (StringPool is shared, vocab is the underlying dictionary)
    cout << "\n[Global/Shared Data]\n";
    cout << "  StringPool Size: " << pool.size() / (1024.0 * 1024.0) << " mB\n";
    cout << "  Vocabulary (HashTable) Size: " << vocab.getSize() << " unique words\n";
}

void switch_index_action() {
    current_index = (current_index == INDEX_HASH) ? INDEX_TRIE : INDEX_HASH;
    cout << "\n--- Implementation Switched ---\n";
    cout << "Current Autocomplete Index: "
         << ((current_index == INDEX_HASH) ? "Hash-based Index (PrefixIndex)" : "Trie-based Index (TrieIndex)")
         << ".\n";
}


void display_results(const vector<Candidate>& out, const string& pre, int time_us)
{
    if (!is_data_loaded) return;

    size_t display_count = std::min(out.size(), (size_t)K);

    cout << "\n--- Suggestions for \"" << pre << "\" ---\n";
    if (display_count == 0)
    {
        cout << "No suggestions found.\n";
    }

    for (size_t i = 0; i < display_count; ++i)
    {
        auto& c = out[i];
        // extract word from pool
        const char* word_ptr = pool.ptr(c.off);
        string w(word_ptr, c.len);
        cout << "  " << (i+1) << ". " << w << " (freq=" << c.freq << ")\n";
    }
    cout << "Time: " << (double)time_us / 1000.0 << " ms\n";
}

// main
int main(int argc, char** argv)
{
    std::string path = (argc > 1) ? std::string(argv[1]) : std::string("../data/allwords_wordset.json");
    load_data_action(path);
    current_index = INDEX_HASH;

    while (true)
    {
    std::cout << "\n======================================================\n";
    std::cout << "Current Index: "<< ((current_index == INDEX_HASH) ? "Hash-based (PrefixIndex)" : "Trie-based (TrieIndex)")<< " | Data: " << (is_data_loaded ? "LOADED" : "UNLOADED") << "\n";
    std::cout << "======================================================\n";
    std::cout << "[1] Load Dataset (" << path << ")\n";
    std::cout << "[2] Build Structure: (a) Hash-Index (b) Trie-Index\n";
    std::cout << "[3] Query Prefix: k=" << K << "\n";
    std::cout << "[4] Inspect stats (Memory, Build time)\n";
    std::cout << "[5] Switch implementation (current: "<< ((current_index == INDEX_HASH) ? "Hash" : "Trie") << ")\n";
    std::cout << "[6] Exact Word Lookup\n";
    std::cout << "[0] Exit\n> ";

    int ch;
//error check inputs
    if (!(std::cin >> ch)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid input. Please enter a number from the menu.\n";
        continue;
    }
    // spacing/display
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (ch == 0) break;
    //menu option cases
    switch (ch) {
      case 1:
        load_data_action(path);
        break;
      case 2:
        build_structure_action();
        break;
      case 3:
        query_prefix_action();
        break;
      case 4:
        display_stats_action();
        break;
      case 5:
        switch_index_action();
        break;
      case 6:
        exact_lookup_action();
        break;
      default:
        cout << "Invalid choice. Please enter a number between 0 and 6.\n";
    }
  }

  cout << "\nExiting Autocomplete Demo. Goodbye!\n";
  return 0;
}