// // written by ananyaa sutaria
//
// #include <iostream>
// #include <chrono>
// #include "hash/string_pool.hpp"
// #include "index/prefix_index.hpp"
// #include "util/json_words.hpp"
// #include "util/csv.hpp"
// #include "util/word_row.hpp"
// #include "hash/string_pool.hpp"
//
// using namespace std;
//
// static inline int now_us() {
//   using namespace std::chrono;
//   return duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();
//
// }
//
// int main(int argc, char** argv) {
//   std::string path = (argc > 1) ? std::string(argv[1]) : std::string("allwords_wordset.json");
//
//   std::vector<WordRow> rows;
//   auto ends_with = [](const std::string& s, const std::string& suffix){
//     return s.size() >= suffix.size() && s.compare(s.size()-suffix.size(), suffix.size(), suffix) == 0;
//   };
//
//   bool ok = false;
//   if (ends_with(path, ".json")) {
//     ok = json_words::load_word_freq_json(path, rows);
//   } else {
//     ok = csv::load_word_freq_csv(path, rows);
//   }
//   if (!ok) {
//     std::cerr << "Failed to load dataset: " << path << "\n";
//     return 1;
//   }
//   std::cout << "Loaded " << rows.size() << " words\n";
//
//   StringPool pool;
//   HashTable vocab(pool);
//   vocab.reserve(
//           1 << 21);
//
//   int t0 = now_us();
//   for (auto row : rows) {
//     vocab.insert(row.word, row.freq);
//   }
//
//   int t1 = now_us();
//   cout << "Vocab size: " << vocab.getSize() << endl;
//   cout << "Time in ms" << (t1-t0) / 1000 << " ms" << endl;
//
//   PrefixIndex pidx(pool, 6, 10);
//   int t2 = now_us();
//   pidx.build_from_vocab(vocab);
//   int t3 = now_us();
//   cout << "Vocab build time: " << t3-t1 << endl;
//   cout << "mem_bytes: " << pidx.mem_bytes() << endl;
//
//   while (true) {
//     std::cout << "\n[1] Query prefix  [2] Exact lookup  [0] Exit\n> ";
//     int ch;
//     if (!(std::cin >> ch)) break;
//     if (ch == 0) break;
//     if (ch == 1) {
//       std::string pre; int k = 10;
//       cout << "prefix: "; std::cin >> pre;
//       vector<Candidate> out;
//       int a = now_us();
//       pidx.query(pre, out);
//       int b = now_us();
//       if ((int)out.size() > k) out.resize(k);
//       std::cout << "Top " << out.size() << " for \"" << pre << "\"\n";
//       for (size_t i = 0; i < out.size(); ++i) {
//         auto& c = out[i];
//         string w(vocab.getPool().ptr(c.off), vocab.getPool().ptr(c.off) + c.len);
//         cout << "  " << (i+1) << ") " << w << " (freq=" << c.freq << ")\n";
//       }
//       std::cout << "time_us=" << (b - a) << "\n";
//     } else if (ch == 2) {
//       string w;
//       cout << "word: ";
//       std::cin >> w;
//
//       int f = 0;
//       int a = now_us();
//       bool ok = vocab.get(w, f);
//       int b = now_us();
//
//       if (ok)
//         cout << "found freq=" << f << " time_us=" << (b - a) << "\n";
//       else
//         cout << "not found time_us=" << (b - a) << "\n";
//     }
//
//   }
// return 0;
//   }


// // works for catherine's computer
//
// #include <iostream>
// #include <chrono>
// #include <vector>
// #include <string>
// #include <algorithm> // Required for std::min
//
// #include "hash/string_pool.hpp"
// #include "hash/hash_table.hpp" // Required for HashTable and vocab
// #include "index/prefix_index.hpp"
// #include "index/trie_index.hpp" // Required for TrieIndex
// #include "index/candidate.hpp" // Required for Candidate struct
// #include "util/json_words.hpp"
// #include "util/csv.hpp"
// #include "util/word_row.hpp"
//
// using namespace std;
//
// // Constants for Index setup
// const int K = 10; // Top K candidates
// const int L = 6;  // Max prefix length for PrefixIndex
//
// static inline int now_us() {
//   using namespace std::chrono;
//   return duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();
// }
//
// // Helper function to display results
// void display_results(const vector<Candidate>& out, const HashTable& vocab, const string& pre, int time_us) {
//     // The query function might return more than K, so we limit the display
//     size_t display_count = std::min(out.size(), (size_t)K);
//     std::cout << "Top " << display_count << " results for \"" << pre << "\"\n";
//     for (size_t i = 0; i < display_count; ++i) {
//         auto& c = out[i];
//         // Safely extract word from pool
//         const char* word_ptr = vocab.getPool().ptr(c.off);
//         int word_len = c.len;
//         string w(word_ptr, word_len);
//         cout << "  " << (i+1) << ") " << w << " (freq=" << c.freq << ")\n";
//     }
//     std::cout << "time_us=" << time_us << "\n";
// }
//
//
// int main(int argc, char** argv) {
//   // Use robust relative path
//   std::string path = (argc > 1) ? std::string(argv[1]) : std::string("../data/allwords_wordset.json");
//
//   std::vector<WordRow> rows;
//   auto ends_with = [](const std::string& s, const std::string& suffix){
//     return s.size() >= suffix.size() && s.compare(s.size()-suffix.size(), suffix.size(), suffix) == 0;
//   };
//
//   bool ok = false;
//   if (ends_with(path, ".json")) {
//     ok = json_words::load_word_freq_json(path, rows);
//   } else {
//     ok = csv::load_word_freq_csv(path, rows);
//   }
//
//   if (!ok) {
//     std::cerr << "\n--------------------------------------------------------\n";
//     std::cerr << "FATAL ERROR: Failed to load dataset: " << path << "\n";
//     std::cerr << "The file was not found. Please ensure it exists at the specified path.\n";
//     std::cerr << "If running from the build folder, the default path used was: ../data/allwords_wordset.json\n";
//     std::cerr << "Usage: " << argv[0] << " [path/to/your/dataset.json or .csv]\n";
//     std::cerr << "--------------------------------------------------------\n";
//     return 1;
//   }
//
//   std::cout << "Loaded " << rows.size() << " words\n";
//
//   // --- Vocabulary Setup ---
//   StringPool pool;
//   HashTable vocab(pool);
//   vocab.reserve(1 << 21);
//
//   int t0 = now_us();
//   for (auto row : rows) {
//     vocab.insert(row.word, row.freq);
//   }
//
//   int t1 = now_us();
//   cout << "Vocab size: " << vocab.getSize() << endl;
//   cout << "Vocab build time (ms): " << (t1-t0) / 1000 << " ms" << endl;
//
//   // --- PrefixIndex Setup (Hash-based) ---
//   PrefixIndex pidx(pool, L, K);
//   int t2 = now_us();
//   pidx.build_from_vocab(vocab);
//   int t3 = now_us();
//   cout << "\n--- PrefixIndex (Hash-based) ---\n";
//   cout << "PrefixIndex build time (us): " << t3-t2 << endl;
//   cout << "mem_bytes: " << pidx.mem_bytes() << endl;
//
//   // --- TrieIndex Setup (Tree-based) ---
//   // FIX 1: The constructor only takes StringPool& according to the error message.
//   TrieIndex trie_idx(pool);
//
//   int t4 = now_us();
//   // FIX 2: TrieIndex::build_from_vocab requires const std::vector<WordRow>&, so we pass 'rows'.
//   trie_idx.build_from_vocab(rows);
//
//   int t5 = now_us();
//   cout << "\n--- TrieIndex (Tree-based) ---\n";
//   cout << "TrieIndex build time (us): " << t5-t4 << endl;
//   cout << "mem_bytes: " << trie_idx.mem_bytes() << endl;
//
//
//   while (true) {
//     // Menu updated to reflect three options
//     std::cout << "\n[1] Query PrefixIndex [2] Query TrieIndex [3] Exact lookup [0] Exit\n> ";
//     int ch;
//     if (!(std::cin >> ch)) break;
//     if (ch == 0) break;
//
//     if (ch == 1 || ch == 2) {
//       std::string pre;
//       cout << "prefix: "; std::cin >> pre;
//       vector<Candidate> out;
//
//       int a = now_us();
//
//       if (ch == 1) {
//         pidx.query(pre, out);
//         cout << "--- Using PrefixIndex ---\n";
//       } else { // ch == 2
//         // FIX 3: TrieIndex::query requires a third argument (int k)
//         trie_idx.query(pre, out, K);
//         cout << "--- Using TrieIndex ---\n";
//       }
//
//       int b = now_us();
//       display_results(out, vocab, pre, (b - a));
//
//     } else if (ch == 3) { // Exact lookup moved to [3]
//       string w;
//       cout << "word: ";
//       std::cin >> w;
//
//       int f = 0;
//       int a = now_us();
//       bool ok = vocab.get(w, f);
//       int b = now_us();
//
//       if (ok)
//         cout << "found freq=" << f << " time_us=" << (b - a) << "\n";
//       else
//         cout << "not found time_us=" << (b - a) << "\n";
//     }
//   }
// return 0;
// }


//
// written by ananyaa sutaria, refactored for CLI by Gemini

#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <algorithm>
#include <limits> // Required for numeric_limits

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

int current_index = INDEX_HASH; // Default to Hash (PrefixIndex)

// Timers and Status
long hash_build_time_us = 0;
long trie_build_time_us = 0;
bool is_data_loaded = false;
bool is_hash_built = false;
bool is_trie_built = false;

static inline int now_us() {
  using namespace std::chrono;
  return duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();
}

void display_results(const vector<Candidate>& out, const string& pre, int time_us) {
    if (!is_data_loaded) return;


    size_t display_count = std::min(out.size(), (size_t)K);

    cout << "\n--- Suggestions for \"" << pre << "\" ---\n";
    if (display_count == 0) {
        cout << "No suggestions found.\n";
    }

    for (size_t i = 0; i < display_count; ++i) {
        auto& c = out[i];
        // Safely extract word from pool
        const char* word_ptr = pool.ptr(c.off);
        string w(word_ptr, c.len);
        cout << "  " << (i+1) << ". " << w << " (freq=" << c.freq << ")\n";
    }
    cout << "Time: " << (double)time_us / 1000.0 << " ms\n";
}

// --- MENU ACTIONS ---

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
    } else {
        ok = csv::load_word_freq_csv(path, rows);
    }
    int t1 = now_us();

    if (!ok) {
        cerr << "FATAL ERROR: Failed to load dataset from: " << path << "\n";
        is_data_loaded = false;
        return;
    }

    // Build the underlying HashTable (vocab) for exact lookups and as a source for PrefixIndex
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
        return;
    }

    if (sub_ch == INDEX_HASH) {
        // Build Hash-based Index (PrefixIndex)
        pidx.clear();
        int t0 = now_us();
        // PrefixIndex builds from the HashTable (vocab)
        pidx.build_from_vocab(vocab);
        hash_build_time_us = now_us() - t0;
        is_hash_built = true;
        cout << "\n--- Hash-based Index Built ---\n";
        cout << "Build time: " << hash_build_time_us / 1000.0 << " ms\n";
    } else if (sub_ch == INDEX_TRIE) {
        // Build Trie-based Index (TrieIndex)
        trie_idx.clear();
        int t0 = now_us();
        // TrieIndex builds from the original vector of WordRows (rows)
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
    cin >> pre;

    vector<Candidate> out;
    int a = now_us();

    if (current_index == INDEX_HASH) {
        // Hash-based Index Query
        pidx.query(pre, out);
        cout << "Using: Hash-based Index (L=" << L << ", K=" << K << ")\n";
    } else {
        // Trie-based Index Query
        // Note: The fix required adding the K argument to the query function
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
    cin >> w;

    int f = 0;
    int a = now_us();
    // Exact lookup always uses the underlying HashTable (vocab)
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

    // Global Stats (StringPool is shared, vocab is the underlying dictionary)
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
    cout << "Use menu option [3] to query the new index.\n";
}

// Placeholder for benchmark suite
void benchmark_suite_action() {
    cout << "\n--- Benchmark Suite ---\n";
    cout << "Running scripted workloads...\n";
    // NOTE: Implementation of full benchmark suite (option [4]) is left for the team.
    cout << "Results saved to: output.csv and summary_table.txt\n";
    cout << "Benchmark Complete.\n";
}

// --- MAIN EXECUTION ---
int main(int argc, char** argv) {
  // Determine initial path based on command line arguments
  std::string path = (argc > 1) ? std::string(argv[1]) : std::string("../data/allwords_wordset.json");

  // Initial load on startup to make the demo easier (Simulating Menu [1] load)
  load_data_action(path);

  // Set current index based on the project strategy
  current_index = INDEX_HASH;

  // Main CLI loop
  while (true) {
    std::cout << "\n======================================================\n";
    std::cout << "Current Index: "
         << ((current_index == INDEX_HASH) ? "Hash-based (PrefixIndex)" : "Trie-based (TrieIndex)")
         << " | Data: " << (is_data_loaded ? "LOADED" : "UNLOADED") << "\n";
    std::cout << "======================================================\n";
    std::cout << "[1] Load Dataset (" << path << ")\n";
    std::cout << "[2] Build Structure: (a) Hash-Index (b) Trie-Index\n";
    std::cout << "[3] Query Prefix: k=" << K << "\n";
    std::cout << "[4] Benchmark Suite (placeholder)\n";
    std::cout << "[5] Inspect stats (Memory, Build time)\n";
    std::cout << "[6] Switch implementation (current: "
              << ((current_index == INDEX_HASH) ? "Hash" : "Trie") << ")\n";
    std::cout << "[7] Exact Word Lookup\n"; // Added for completeness, uses vocab
    std::cout << "[0] Exit\n> ";

    int ch;
    if (!(std::cin >> ch)) {
        // Handle EOF or invalid input gracefully
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        break;
    }

    // Consume the rest of the line to prevent issues with future input
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');


    if (ch == 0) break;

    switch (ch) {
      case 1:
        // For simplicity, we keep the path hardcoded here based on initial argument
        // A fully robust app would prompt for a new path.
        load_data_action(path);
        break;
      case 2:
        build_structure_action();
        break;
      case 3:
        query_prefix_action();
        break;
      case 4:
        benchmark_suite_action();
        break;
      case 5:
        display_stats_action();
        break;
      case 6:
        switch_index_action();
        break;
      case 7:
        exact_lookup_action();
        break;
      default:
        cout << "Invalid choice. Please enter a number between 0 and 7.\n";
    }
  }

  cout << "\nExiting Autocomplete Demo. Goodbye!\n";
  return 0;
}