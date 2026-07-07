#include "echotrace/fs_traversal.hpp"

#include "echotrace/text.hpp"

#include <algorithm>
#include <filesystem>
#include <stdexcept>
#include <system_error>
#include <unordered_set>

namespace echotrace
{

namespace fs
{

namespace stdfs = std::filesystem;

namespace
{

constexpr std::string_view kSupportedExtensions[] = {
    ".cpp", ".cc", ".cxx", ".h", ".hpp"
};

// Walk the directory tree, pruning ignored subtrees, appending matching files.
void walk(const stdfs::path& root,
          const stdfs::path& base,
          const DiscoveryOptions& options,
          std::vector<DiscoveredFile>& out)
{
    std::error_code ec;
    for (auto it = stdfs::directory_iterator(root, ec); !ec && it != stdfs::directory_iterator(); it.increment(ec))
    {
        const auto& entry = *it;
        const auto name = entry.path().filename().string();

        if (entry.is_directory(ec))
        {
            if (is_ignored_directory(name))
            {
                continue; // prune: never descend
            }
            walk(entry.path(), base, options, out);
        }
        else if (entry.is_regular_file(ec) || entry.is_symlink(ec))
        {
            if (!has_supported_extension(name))
            {
                continue;
            }

            DiscoveredFile file;
            file.path = entry.path().string();

            auto rel = std::filesystem::relative(entry.path(), base, ec);
            file.display = ec ? name : rel.string();

            out.push_back(std::move(file));
        }
    }

    if (ec)
    {
        throw std::runtime_error("directory traversal failed under '" + root.string() +
                                 "': " + ec.message());
    }
}

} // namespace

bool has_supported_extension(const std::string& filename)
{
    const auto lower = text::to_lower(filename);
    for (auto ext : kSupportedExtensions)
    {
        if (lower.size() >= ext.size() &&
            lower.compare(lower.size() - ext.size(), ext.size(), ext) == 0)
        {
            return true;
        }
    }
    return false;
}

bool is_ignored_directory(const std::string& dir_name)
{
    if (dir_name.empty())
    {
        return false;
    }
    // Any hidden (dot) directory, including .git and hidden temp dirs.
    if (dir_name[0] == '.')
    {
        return true;
    }
    if (dir_name == "build" || dir_name == "out" || dir_name == "bin")
    {
        return true;
    }
    // cmake-build* (Debug/Release variants produced by CMake/IDE tooling).
    if (dir_name.size() >= 11 && dir_name.compare(0, 11, "cmake-build") == 0)
    {
        return true;
    }
    return false;
}

std::vector<DiscoveredFile> discover_sources(const std::string& root,
                                             const DiscoveryOptions& options)
{
    std::error_code ec;
    const stdfs::path rootPath(root);
    const auto status = stdfs::status(rootPath, ec);
    if (ec)
    {
        throw std::runtime_error("cannot access '" + root + "': " + ec.message());
    }

    std::vector<DiscoveredFile> out;

    if (stdfs::is_directory(status))
    {
        walk(rootPath, rootPath, options, out);
    }
    else if (stdfs::is_regular_file(status))
    {
        // A single file: include it only if it has a supported extension.
        const auto name = rootPath.filename().string();
        if (has_supported_extension(name))
        {
            out.push_back({root, name});
        }
        else
        {
            throw std::runtime_error("'" + root + "' is not a C++ source file");
        }
    }
    else
    {
        throw std::runtime_error("'" + root + "' is neither a file nor a directory");
    }

    if (options.deduplicate)
    {
        // Collapse paths that resolve to the same canonical file (symlinks).
        std::unordered_set<std::string> seen;
        std::vector<DiscoveredFile> unique;
        unique.reserve(out.size());
        for (auto& f : out)
        {
            const auto canon = std::filesystem::weakly_canonical(f.path, ec).string();
            const auto key = ec ? f.path : canon;
            if (seen.insert(key).second)
            {
                unique.push_back(std::move(f));
            }
        }
        out = std::move(unique);
    }

    std::sort(out.begin(), out.end(),
              [](const DiscoveredFile& a, const DiscoveredFile& b)
              { return a.display < b.display; });

    return out;
}

} // namespace fs

} // namespace echotrace
