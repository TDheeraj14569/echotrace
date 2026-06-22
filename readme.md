EchoTrace

A high-performance, localized code similarity detector written in standard C++17.

EchoTrace implements the Winnowing algorithm (the foundational algorithm behind tools like Stanford's MOSS) to detect structural cloning, copied code, and plagiarism. By evaluating the underlying AST-like structure of the code rather than exact text matches, it successfully detects copies even when variables have been renamed, data types altered, or whitespace heavily modified.

Technical Highlights

This project was built with a focus on algorithmic efficiency and memory safety:

Zero-Copy Parsing: The custom lexer heavily utilizes std::string_view to traverse source files without triggering heap allocations, keeping memory overhead strictly bound to the final fingerprint generation.

$O(N)$ Fingerprinting: The winnowing sliding window is implemented using a monotonically increasing std::deque. This ensures that local minimum hashes are found in $O(N)$ time, avoiding the $O(N \times W)$ complexity of naive sliding window implementations.

Semantic Tokenization: Identifiers are mapped to generic V (Variable) tokens, and literals to N (Number) or L (Literal) tokens. This neutralizes common plagiarism tactics like bulk variable renaming.

Dependency-Free: Built entirely using the C++ Standard Library. No external libraries or build systems are required.

The Pipeline

EchoTrace processes files in four distinct phases:

Lexical Analysis: Source code is stripped of comments/whitespace and converted into a stream of structural tokens.

$k$-gram Generation: Tokens are grouped into overlapping windows of size $k$.

Rolling Hashes: $k$-grams are hashed using a polynomial rolling hash function.

Winnowing: A subset of these hashes (the "fingerprint") is selected using a sliding window of size $w$ to guarantee that matches of a certain length are always detected while minimizing the data stored.

Evaluation: The Jaccard Index is calculated between the two fingerprints to determine the percentage of structural overlap.

Getting Started

Prerequisites

A C++ compiler supporting C++17 or higher (GCC, Clang, or MSVC).

Building

Compile the source code with maximum optimization (-O3):

g++ -std=c++17 -O3 echotrace.cpp -o echotrace


Verification

Run the built-in test suite to verify the internal logic and algorithm accuracy:

./echotrace --self-test


Usage

Compare two source files using the default parameters ($k=5$, $w=4$):

./echotrace fileA.cpp fileB.cpp


Advanced Configuration

You can tune the sensitivity of the algorithm using command-line flags:

-k (k-gram size): Defines the noise threshold. Larger values ignore smaller copied snippets. (Default: 5)

-w (window size): Defines the gap tolerance between matching fingerprints. (Default: 4)

./echotrace fileA.cpp fileB.cpp -k 10 -w 5


Example Output:

EchoTrace file comparison
-------------------------
k-gram size:       5
winnow window:     4

Left: fileA.cpp
  tokens:      342
  k-grams:     338
  fingerprints: 85
Right: fileB.cpp
  tokens:      342
  k-grams:     338
  fingerprints: 85

Similarity: 100.00%


Future Roadmap

Containment Metric: Transitioning from Jaccard Similarity to a Containment metric to better handle scenarios where a small copied snippet is hidden within a massive codebase.

Directory Scanning: Adding support for batch processing entire directories of student submissions, generating an $N \times N$ similarity matrix.

Preprocessor Stripping: Adding lexer rules to ignore #include and #define directives to prevent boilerplate code from artificially inflating similarity scores.