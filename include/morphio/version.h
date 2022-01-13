#pragma once

#include <cstdint>  // std::uint32_t
#include <string>
#include <utility>  // std::tuple

namespace morphio {

// A tuple (file format (std::string), major version, minor version)
using MorphologyVersion = std::tuple<std::string, uint32_t, uint32_t>;

std::string getVersionString();
int getMajor();
int getMinor();
int getPatch();


}  // namespace morphio
