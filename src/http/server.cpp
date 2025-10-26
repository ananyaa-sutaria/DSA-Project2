//written by ananyaa sutaria

#include <algorithm>
#include <string>
#include <vector>
#include <cctype>

#include "third_party/httplib.h"
#include "util/word_row.hpp"
#include "util/json_words.hpp"
#include "util/json_words.hpp"

#include "hash/string_pool.hpp"
#include "hash/hash_table.hpp"
#include "index/prefix_index.hpp"

using namespace std;

static inline string lower_ascii(const string &s) {
    string t = s;
    for (char &c :t ) if (c >= 'A' && c <= 'Z')
        c = char (c - 'A' + 'a');
    return t;
}

static StringPool g_pool;
static HashTable* g_vocab = nullptr;
static PrefixIndex* g_pidx;

static string json_escape(const char* s, int len) {
    string out;
    out.reserve(len + 8);
    for (int i = 0; i < len; ++i) {
        unsigned char c= (unsigned char)s[i];
        if (c == '"' || c == '\\') {
            out.push_back('\\');
            out.push_back((char) c);
        } else if (c == '\n') {
            out += "\\n";
        } else if (c == '\r') {
            out += "\\r";
        } else if (c == '\t') {
            out += "\\t";
        } else {
            out.push_back((char)c);
        }
        return out;

    }
}

static string to_json (const vector<Candidate>& items) {
    string j = "[";
    for (size_t i = 0; i < items.size(); ++i) {
        const auto& c = items [i];
        const char* p =g_pool.ptr(c.off);
        string w = json_escape(p, c.len);
        j+= "{\"word\":\"" + w + "\",\"freq\":" + std::to_string(c.freq) + "}";
        if (i + 1 < items.size())
            j += ",";
    }
    j += "]";
    return j;
}

int main (int argc, char** argv) {
    string path = (argc > 1) ? string(argv[1]) : string("data/allwords_woordset.json");

    vector<WordRow> rows;
    bool valid = json_words::load_word_freq_json(path, rows);
    if (!valid) {
        fprintf(stderr, "Failed to load data set: ", path.c_str());
        return 1;
    }
    fprintf(stdout, "Loaded %zu words\n", rows.size());

    g_vocab = new HashTable(g_pool);
    g_vocab->reserve(1 << 21);
    for (const auto &row: rows) {
        g_vocab->insert(row.word, row.freq);
    }

    g_pidx = new PrefixIndex(g_pool, 6, 10);
    g_pidx->build_from_vocab(*g_vocab);
    fprintf(stdout, "Prefix index readt\n");

    httplib::Server sv;

    sv.set_default_headers({
                                   {"Access-Contorl-Allow-Origin",  "*"},
                                   {"Access-Control-Allow-Headers", "content-type"},
                           });

    sv.Get("/query", [](const httplib::Request &req, httplib::Response &res) {
        auto it = req.get_param_value("q");
        std::string q = lower_ascii(it);
        int k = 10;
        if (auto kp = req.get_param_value("k"); !kp.empty()) {
            try { k = std::max(1, std::min(50, std::stoi(kp))); } catch (...) {}
        }

        std::vector<Candidate> out;
        g_pidx->query(q, out);
        if ((int) out.size() > k) out.resize(k);

        res.set_content(to_json(out), "application/json");
    });

    sv.Get("/", [](const httplib::Request&, httplib::Response& res) {
        // very small, embedded HTML (you can also serve a file if you prefer)
        static const char* html = R"HTML(
<!doctype html>
<html>
<head>
  <meta charset="utf-8" />
  <title>Autocomplete</title>
  <style>
    body { font: 16px system-ui, -apple-system, Helvetica, Arial; padding: 24px; }
    .box { max-width: 640px; margin: 40px auto; }
    input { width: 100%; padding: 12px 14px; border: 1px solid #ddd; border-radius: 10px; font-size: 18px; }
    ul { list-style: none; padding: 0; margin: 12px 0 0; border: 1px solid #eee; border-radius: 10px; }
    li { padding: 10px 12px; border-bottom: 1px solid #f3f3f3; }
    li:last-child { border-bottom: none; }
    .freq { color: #888; font-size: 12px; margin-left: 8px; }
  </style>
</head>
<body>
  <div class="box">
    <h2>Autocomplete</h2>
    <input id="q" placeholder="type a prefix… e.g. trans" autofocus />
    <ul id="list"></ul>
  </div>
<script>
  const q = document.getElementById('q');
  const list = document.getElementById('list');

  let timer = null;
  q.addEventListener('input', () => {
    clearTimeout(timer);
    timer = setTimeout(async () => {
      const v = q.value.trim();
      if (!v) { list.innerHTML = ''; return; }
      const r = await fetch(`/query?q=${encodeURIComponent(v)}&k=10`);
      const arr = await r.json();
      list.innerHTML = arr.map((x, i) =>
        `<li>${i+1}. ${x.word} <span class="freq">freq=${x.freq}</span></li>`).join('');
    }, 120);
  });
</script>
</body>
</html>
)HTML";
        res.set_content(html, "text/html; charset=utf-8");
    });

    printf("Server on http://127.0.0.1:8080  (open in your browser)\n");
    sv.listen("0.0.0.0", 8080);
    return 0;
}