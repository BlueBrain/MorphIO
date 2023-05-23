#pragma once
#include <clocale>  // locale_t

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
    int64_t toInt(const char* pos, const char** endpos, int base) const;
    floatType toFloat(const char* pos, const char** endpos) const;
};

StringToNumber& getStringToNumber();

}  // namespace morphio
