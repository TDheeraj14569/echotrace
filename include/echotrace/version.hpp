#ifndef ECHOTRACE_VERSION_HPP
#define ECHOTRACE_VERSION_HPP

/// EchoTrace semantic version. Bumped with each release.
#define ECHOTRACE_VERSION_MAJOR 1
#define ECHOTRACE_VERSION_MINOR 0
#define ECHOTRACE_VERSION_PATCH 0

namespace echotrace
{

/// Human-readable version string, e.g. "1.0.0".
inline constexpr const char* kVersion = "1.0.0";

} // namespace echotrace

#endif // ECHOTRACE_VERSION_HPP
