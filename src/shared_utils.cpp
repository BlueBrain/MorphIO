#include "shared_utils.hpp"

#include "../3rdparty/filesystem.hpp"

namespace morphio {

bool is_directory(const std::string& path) {
    return ghc::filesystem::is_directory(ghc::filesystem::canonical(path));
}

bool is_regular_file(const std::string& path) {
    return ghc::filesystem::is_regular_file(ghc::filesystem::canonical(path));
}

std::string join_path(const std::string& dirname, const std::string& filename) {
    return (ghc::filesystem::path(dirname) / filename).string();
}

}  // namespace morphio
