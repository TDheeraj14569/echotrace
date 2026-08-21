# EchoTrace

**Source-code similarity detection engine for academic integrity and code auditing.**

EchoTrace analyzes collections of source code files to detect similarity using document fingerprinting — the same algorithmic family used by MOSS (Stanford) and JPlag. It tokenizes, normalizes, hashes, and fingerprints source files, then computes Jaccard similarity across all pairs.

## Supported Languages

| Language | Extensions | Status |
|----------|-----------|--------|
| C/C++ | `.c` `.h` `.cpp` `.cc` `.cxx` `.hpp` | ✅ Full support |
| Python | `.py` `.pyw` | ✅ Full support |
| Java | `.java` | ✅ Full support |
| JavaScript/TypeScript | `.js` `.jsx` `.ts` `.tsx` | ✅ Full support |

## How It Works

```
Source files
    │
    ▼
┌─────────────┐   Language-specific tokenization:
│   Lexer     │   identifiers → V, types → T, numbers → N, strings → L
└──────┬──────┘   keywords preserved verbatim, comments/whitespace stripped
       │
       ▼
┌─────────────┐   Redundant parentheses collapsed:
│ Normalizer  │   ((x)) → (x)
└──────┬──────┘
       │
       ▼
┌─────────────┐   Polynomial rolling hash (base=256, mod=10^9+7)
│   Hasher    │   over k-grams of token symbols
└──────┬──────┘
       │
       ▼
┌─────────────┐   Winnowing algorithm: selects minimum hash
│ Fingerprint │   in each sliding window → robust, position-independent
└──────┬──────┘
       │
       ▼
┌─────────────┐   Jaccard similarity: |A ∩ B| / |A ∪ B|
│ Comparator  │   over fingerprint sets, parallel via thread pool
└──────┬──────┘
       │
       ▼
  Report (Text, CSV, JSON, HTML)
```

## Quick Start

### Prerequisites
- **C++20 compiler**: GCC 13+, Clang 16+, or MSVC 19.35+
- **Node.js 18+** (for web interface)
- **Make** or **CMake** (for building)

### Build the Engine

```bash
# Linux / macOS
make cli

# Windows (PowerShell)
.\build.ps1 cli
```

### Run from Command Line

```bash
# Compare all files in a directory
./build/bin/echotrace ./submissions/ --format text

# Compare two specific files
./build/bin/echotrace fileA.cpp fileB.cpp

# With custom parameters
./build/bin/echotrace ./code/ --k 7 --window 5 --threshold 30 --format html -o report.html

# Run self-test
./build/bin/echotrace --self-test
```

### CLI Options

| Flag | Default | Description |
|------|---------|-------------|
| `--format` | `text` | Output format: `text`, `csv`, `json`, `html` |
| `--k` | `5` | K-gram size for hashing |
| `--window` | `4` | Winnowing window size |
| `--threshold` | `0` | Minimum similarity % to report |
| `--threads` | `auto` | Worker threads (0 = auto-detect) |
| `--top` | `0` | Show only top-N matches |
| `-o` | stdout | Output file path |

### Run Unit Tests

```bash
make test          # Linux/macOS
.\build.ps1 tests  # Windows
```

### Run Benchmarks

```bash
make bench
```

## Web Interface

EchoTrace includes a full web application for browser-based analysis.

### Setup

```bash
# Install backend dependencies
cd web/backend && npm install

# Install frontend dependencies
cd web/frontend && npm install

# Start backend (port 3001)
cd web/backend && npm start

# Start frontend dev server (port 5173)
cd web/frontend && npm run dev
```

### API Endpoints

| Method | Endpoint | Description |
|--------|----------|-------------|
| `POST` | `/api/upload` | Upload source files |
| `POST` | `/api/analysis/:id/start` | Start analysis |
| `GET` | `/api/analysis/:id/status` | Check progress |
| `GET` | `/api/results/:id` | Get results |
| `GET` | `/api/results` | List all jobs |
| `DELETE` | `/api/results/:id` | Delete a job |
| `GET` | `/api/health` | Health check |

### Docker

```bash
docker-compose up --build
# Access at http://localhost:3001
```

## Architecture

```
echotrace/
├── include/echotrace/     # Public headers
│   ├── lexer.hpp          # C++ tokenizer
│   ├── language.hpp       # Language detection
│   ├── language_lexer.hpp # Abstract lexer interface
│   ├── normalization.hpp  # Token normalization
│   ├── hash.hpp           # Rolling hash (FNV-1a + polynomial)
│   ├── fingerprint.hpp    # Winnowing algorithm
│   ├── document.hpp       # ParsedSource aggregate type
│   ├── similarity.hpp     # Jaccard similarity
│   ├── fragments.hpp      # Matched fragment extraction
│   ├── analysis.hpp       # Multi-file analysis pipeline
│   ├── indexing.hpp       # Inverted fingerprint index
│   ├── reporting.hpp      # Report rendering
│   └── thread_pool.hpp    # Header-only thread pool
├── src/                   # Implementations
│   ├── languages/         # Multi-language lexers
│   │   ├── cpp_lexer.cpp
│   │   ├── python_lexer.cpp
│   │   ├── java_lexer.cpp
│   │   └── javascript_lexer.cpp
│   └── cli/main.cpp       # CLI entry point
├── tests/                 # Unit tests (87 tests)
├── bench/                 # Benchmark harness
├── web/
│   ├── backend/           # Express REST API
│   └── frontend/          # React + TypeScript UI
├── Makefile               # GNU Make build
├── build.ps1              # Windows PowerShell build
├── Dockerfile             # Multi-stage Docker build
└── docker-compose.yml     # Docker Compose
```

### Key Design Decisions

- **C++ engine stays the computational core.** The web backend invokes the CLI as a subprocess — no FFI complexity.
- **Same-language comparison only.** Cross-language similarity (e.g., C++ vs Python) is not meaningful with token-level fingerprinting.
- **O(N·candidates) via inverted index.** For large file sets, the indexed comparison mode (`ComparisonMode::Indexed`) prunes pairs that share no fingerprints, avoiding O(N²) comparisons.
- **Lock-free parallel comparison.** Each thread writes to its own pre-assigned result slot — no mutexes needed.

## Performance

Benchmarks on a modern desktop (AMD Ryzen, single-threaded unless noted):

| Files | Lines/File | Tokens | Pairs | Time |
|-------|-----------|--------|-------|------|
| 10 | 50 | 4,855 | 45 | 2.7ms |
| 50 | 100 | 51,820 | 1,225 | 19ms |
| 100 | 100 | 102,723 | 4,950 | 38ms |
| 200 | 50 | 101,791 | 19,900 | 81ms (4T) |

## License

MIT — Copyright 2026 Thota Dheerajeswar
