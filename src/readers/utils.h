#pragma once
#include <clocale>  // locale_t
#include <string>
#include <utility> // std::tuple

#include <morphio/vector_types.h>  // floatType

namespace morphio {

struct StringToNumber {
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    _locale_t locale;
#else
    locale_t locale;
#endif

    StringToNumber();
    ~StringToNumber();

    std::tuple<int64_t, size_t> toInt(const std::string&s, size_t offset) const;
    std::tuple<floatType, size_t> toFloat(const std::string&s, size_t offset) const;
};

StringToNumber& getStringToNumber();

}  // namespace morphio
