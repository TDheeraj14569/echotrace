# EchoTrace

A high-performance, localized code similarity detector written in standard C++20.

EchoTrace implements the **Winnowing algorithm** — the same class of algorithm behind tools like Stanford's MOSS — to detect structural cloning and plagiarism across C++ source trees. By comparing normalized token streams rather than raw text, it detects copied logic even when identifiers, literals, comments, and formatting have all been changed.

This started as a single-file prototype (`Moss-Lite`) and has since been rebuilt from the ground up as a proper library + CLI: a real `src/`/`include/` split, a multithreaded comparison engine, whole-directory batch analysis, and four report formats.

## Technical Highlights

- **Zero-Copy Parsing** — the lexer runs entirely on `std::string_view` / byte offsets, so tokenizing a file allocates no intermediate substrings. Each token retains its original source span, which is what makes matched-fragment highlighting possible later in the pipeline.

- **Semantic Tokenization** — identifiers collapse to a generic `V` symbol, types to `T`, numeric literals to `N`, and string/char literals to `L`. Keywords and operators are preserved verbatim because they carry the structural meaning the detector actually relies on. Two files that differ only in identifier spelling, literal values, and layout produce identical token streams.

- **Safe, Provable Normalization** — a second pass collapses redundant nested parentheses (`((E))` → `(E)`). Every included transform is *guaranteed* semantics-preserving; anything with an edge case (`a += b` vs `a = a + b`, `++x` vs `x += 1`, folding `while(0)`) is deliberately left out rather than risked.

- **O(N) Fingerprinting** — the winnowing sliding window is implemented with a monotonic `std::deque`, so local-minimum hash selection is O(N) total, not the O(N × W) of a naive sliding-window scan.

- **Parallel Pairwise Comparison** — directory mode compares every file pair (N×(N−1)/2 comparisons) across a hand-rolled thread pool. Each worker task owns a pre-assigned output slot, so there's no locking on the hot path and results are byte-identical regardless of thread scheduling.

- **Matched-Fragment Retrieval** — beyond a single similarity percentage, EchoTrace can recover the actual shared k-gram spans between two files (original source offsets, both sides), for reports that show *where* two files overlap, not just *how much*.

- **Dependency-Free** — standard library only. No external packages, no CMake — a single `Makefile` builds everything.

## The Pipeline

```text
Source File
     |
     v
Lexer  ──────────────► normalizes to V / T / N / L symbols, discards comments & whitespace
     |
     v
Token Stream
     |
     v
Redundant-Paren Pass ─► provably safe token-level cleanup
     |
     v
K-Grams  ────────────► overlapping windows of size k
     |
     v
Rolling Hashes ──────► polynomial rolling hash, O(1) amortized per window
     |
     v
Winnowing ───────────► sliding window of size w selects local-minimum hashes
     |
     v
Fingerprint  (a hash set per file)
     |
     v
Jaccard Similarity ──► pairwise, optionally multithreaded across the whole corpus
     |
     v
Ranked Report  (text / csv / json / html)
```

## Project Structure

```text
echotrace/
├── Makefile
├── include/echotrace/
│   ├── lexer.hpp          # tokenizer: source -> normalized token stream
│   ├── normalization.hpp  # provably-safe token-level cleanup passes
│   ├── hash.hpp            # k-gram rolling hash
│   ├── fingerprint.hpp     # winnowing selection
│   ├── similarity.hpp      # Jaccard comparison
│   ├── fragments.hpp       # shared k-gram -> source span recovery
│   ├── document.hpp        # ParsedSource: a file's full pipeline output
│   ├── fs_traversal.hpp    # recursive source discovery
│   ├── analysis.hpp        # N-file pairwise engine, thresholds, top-N
│   ├── reporting.hpp       # text / csv / json / html renderers
│   ├── thread_pool.hpp     # fixed-size worker pool used by analysis
│   └── version.hpp
└── src/
    ├── cli/main.cpp        # argument parsing, two modes, self-test
    └── *.cpp               # implementations matching each header above
```

## Getting Started

### Prerequisites

A C++20 compiler (GCC or Clang) and `make`. No other dependencies.

### Building

```bash
make            # builds ./build/bin/echotrace
```

Other targets:

```bash
make lib        # static library only (libechotrace.a)
make clean      # remove all build output
```

The build compiles under `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion` with zero warnings.

### Verification

Run the built-in fingerprint self-check (exact clone, identifier-renamed clone, and unrelated file — verifies expected similarity in each case):

```bash
./build/bin/echotrace --self-test
```

### Usage

**Directory mode** — recursively discover every `.cpp/.cc/.cxx/.h/.hpp` file under a root and compute a full pairwise similarity matrix:

```bash
./build/bin/echotrace ./submissions
```

**Two-file mode** — compare exactly two files directly:

```bash
./build/bin/echotrace fileA.cpp fileB.cpp
```

### Options

```text
Analysis options:
  -k, --k N                    token k-gram size (default 5)
  -w, --w N                    winnowing window size (default 4)
  -j, --threads N               comparison worker threads (default: auto)
  --threshold PCT, --min PCT    minimum similarity to report (default 0)
  --top N                       keep only the top N matches (default: all)
  --no-normalize                disable redundant-paren normalization

Output options:
  --format text|csv|json|html   report format (default: text; directory mode only)
  -o, --output FILE             write report to FILE instead of stdout
```

- **k (k-gram size)** — the noise threshold. Larger values ignore smaller copied snippets.
- **w (window size)** — the gap tolerance between selected fingerprints; also bounds how sparse the fingerprint set is.

### Example Output (directory mode)

```text
EchoTrace Analysis Report
=========================

Root:            ./submissions
Files:                  24
Comparisons:            276
Highest similarity:     100.00%
Average similarity:     6.42%
Matches (>= 0.00%):     276

Ranked Matches
--------------
rank  similarity   file_a          file_b          shared
   1     100.00%   alice/main.cpp  bob/main.cpp        21
   2      41.30%   carol/sort.cpp  dave/sort.cpp        9
```

`--format json` and `--format html` produce the same result as structured data or a self-contained report page, for feeding into a grading pipeline or opening directly in a browser.

## Future Roadmap

- **Containment Metric** — a containment score alongside Jaccard, for cases where a small snippet is copied into a much larger file (Jaccard under-weights this case by design).
- **Preprocessor-Aware Lexing** — teach the lexer to strip `#include`/`#define` directives so shared boilerplate doesn't inflate similarity between otherwise-unrelated files.
- **HTML Diff View** — render matched-fragment spans (already computed by the fragments module) as an inline side-by-side diff in the HTML report, rather than just listing similarity numbers.

## Limitations

- Token-based analysis detects structural/lexical cloning, not deeper semantic plagiarism (e.g. an algorithm rewritten with a genuinely different control-flow shape).
- Heavily templated or generated code can inflate similarity scores, since generated boilerplate tokenizes identically across files.
- Two-file mode is intentionally minimal and does not support the `--format`/`--output` report renderers — use directory mode (even for two files, if you want CSV/JSON/HTML output).
