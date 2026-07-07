#ifndef ECHOTRACE_FS_TRAVERSAL_HPP
#define ECHOTRACE_FS_TRAVERSAL_HPP

#include <string>
#include <vector>

namespace echotrace
{

/// Directory discovery for the multi-file analysis engine.
namespace fs
{

/// A discovered source file: [path] is openable from the current working
/// directory; [display] is the path relative to the scan root, suitable for
/// reports and matrix labels.
struct DiscoveredFile
{
    std::string path;
    std::string display;
};

/// Options controlling directory discovery.
struct DiscoveryOptions
{
    /// When true, files reachable through multiple paths (e.g. symlinks) are
    /// reported once, deduplicated by canonical filesystem identity.
    bool deduplicate = true;
};

/// Recursively discover supported C++ source files under [root].
///
/// Supported extensions: .cpp .cc .cxx .h .hpp (matched case-insensitively).
/// Pruned directories (never descended into):
///   build/  out/  bin/  .git/  cmake-build*/  any hidden (dot) directory
///
/// Throws std::runtime_error if [root] does not exist or is unreadable.
/// Returns an empty vector for an existing but empty directory (not an error).
/// Results are sorted by display path for deterministic output.
///
/// Complexity: O(files) filesystem entries, each visited once; ignored
/// subtrees are pruned and never traversed.
std::vector<DiscoveredFile> discover_sources(const std::string& root,
                                             const DiscoveryOptions& options = {});

/// True if [filename] has a supported C++ source extension.
bool has_supported_extension(const std::string& filename);

/// True if [dir_name] should be pruned during traversal.
bool is_ignored_directory(const std::string& dir_name);

} // namespace fs

} // namespace echotrace

#endif // ECHOTRACE_FS_TRAVERSAL_HPP
