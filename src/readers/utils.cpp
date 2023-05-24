#include "./utils.h"

namespace morphio {
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
StringToNumber::StringToNumber: locale(_create_locale(LC_ALL, "C")) {}

StringToNumber::~StringToNumber() {
    _free_locale(locale);
}

int64_t StringToNumber::toInt(const char* pos, const char** endpos, int base) const {
    return _strtol_l(pos, const_cast<char**>(endpos), base, locale);
}

floatType StringToNumber::toFloat(const char* pos, const char** endpos) const {
    floatType ret = _strtof_l(pos, const_cast<char**>(endpos), locale);
    if (ret == 0 && pos == *endpos) {
        throw std::invalid_argument("could not parse float");
    }
    return ret;
}
#else
StringToNumber::StringToNumber()
    : locale(newlocale(LC_ALL, "C", nullptr)) {}

StringToNumber::~StringToNumber() {
    freelocale(locale);
}

std::tuple<int64_t, size_t> StringToNumber::toInt(const std::string&s, size_t offset) const {
    const size_t base = 10;
    const char *pos = &s[offset];
    const char *endpos = &s[s.size()];
    int64_t ret = strtol_l(pos, const_cast<char**>(&endpos), base, locale);

    auto new_offset = static_cast<size_t>(endpos - s.data());

    if (ret == 0 && new_offset == 0) {
        throw std::invalid_argument("could not parse integer");
    }

    return {ret, new_offset};
}

std::tuple<floatType, size_t> StringToNumber::toFloat(const std::string&s, size_t offset) const {
    const char *pos = &s[offset];
    const char *endpos = &s[s.size()];
    floatType ret = strtof_l(pos, const_cast<char**>(&endpos), locale);

    auto new_offset = static_cast<size_t>(endpos - s.data());

    if (ret == 0 && new_offset == 0) {
        throw std::invalid_argument("could not parse float");
    }

    return {ret, new_offset};
}

#endif

StringToNumber& getStringToNumber() {
    static StringToNumber stn;
    return stn;
}
}  // namespace morphio
