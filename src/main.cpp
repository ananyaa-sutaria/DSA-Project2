// written by ananyaa sutaria

#include <iostream>
#include <chrono>
#include "hash/string_pool.hpp"
#include "hash/hash_table.hpp"
#include "index/prefix_index.hpp"
#include "util/csv.hpp"

using namespace std;

static inline int now_us() {
  using namespace std::chrono;
  return duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();

}

int main(int argc, char** argv) {
  string csv_path =(argc > 1) ? string(argv[1]) : string("data.csv");
  vector<WordRow> rows;
  if(!csv::load_word_freq_csv(csv_path, rows)) {
    cerr << "Failed to load CSV file" << endl;
    return 1;
  }
  cout << "Loaded CSV file" << endl;

  StringPool pool;
  HashTable vocab(pool);
  vocab.reserve(1 << 21);

  int t0 = now_us();
  for (auto row : rows) {
    vocab.insert(row.word, row.frequency);
  }

  int t1 = now_us();
  cout << "Vocab size: " << vocab.getSize() << endl;
  cout << "Time in ms" << (t1-t0) / 1000 << " ms" << endl;

  PrefixIndex pidx(pool, 6, 10);
  int t2 = now_us();
  pidx.build_from_vocab(vocab);
  int t3 = now_us();
  cout << "Vocab build time: " << t3-t1 << endl;
  cout << "mem_bytes: " << pidx.mem_bytes() << endl;

  while (true) {
    std::cout << "\n[1] Query prefix  [2] Exact lookup  [0] Exit\n> ";
    int ch;
    if (!(std::cin >> ch)) break;
    if (ch == 0) break;
    if (ch == 1) {
      std::string pre; int k = 10;
      cout << "prefix: "; std::cin >> pre;
      vector<Candidate> out;
      int a = now_us();
      pidx.query(pre, out);
      int b = now_us();
      if ((int)out.size() > k) out.resize(k);
      std::cout << "Top " << out.size() << " for \"" << pre << "\"\n";
      for (size_t i = 0; i < out.size(); ++i) {
        auto& c = out[i];
        string w(vocab.getPool().ptr(c.off), vocab.getPool().ptr(c.off) + c.len);
        cout << "  " << (i+1) << ") " << w << " (freq=" << c.freq << ")\n";
      }
      std::cout << "time_us=" << (b - a) << "\n";
    } else if (ch == 2) {
      string w;
      cout << "word: ";
      std::cin >> w;

      int f = 0;
      int a = now_us();
      bool ok = vocab.get(w, f);
      int b = now_us();

      if (ok)
        cout << "found freq=" << f << " time_us=" << (b - a) << "\n";
      else
        cout << "not found time_us=" << (b - a) << "\n";
    }

  }
return 0;
  }