#include "./utils.h"

namespace morphio {
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#define freelocale _free_locale
#define strtol_l _strtol_l

#ifdef MORPHIO_USE_DOUBLE
#define strto_float _strtod_l
#else
#define strto_float _strtof_l
#endif

#else  // not WIN32

#ifdef MORPHIO_USE_DOUBLE
#define strto_float strtod_l
#else
#define strto_float strtof_l
#endif

#endif

// Only create the `locale` to facilitate number handling once
StringToNumber::StringToNumber()
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    // On windows, use their locale handling for their runtime
    : locale(_create_locale(LC_ALL, "C")) {
}
#else
    // On other platforms, use the POSIX version
    : locale(newlocale(LC_NUMERIC_MASK, "POSIX", nullptr)) {
}
#endif

StringToNumber::~StringToNumber() {
    freelocale(locale);
}

std::tuple<int64_t, size_t> StringToNumber::toInt(const std::string& s, size_t offset) const {
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

std::tuple<floatType, size_t> StringToNumber::toFloat(const std::string& s, size_t offset) const {
    const char *pos = &s[offset];
    const char *endpos = &s[s.size()];
    floatType ret = strto_float(pos, const_cast<char**>(&endpos), locale);

    auto new_offset = static_cast<size_t>(endpos - s.data());

    if (std::fabs(ret - 0) < morphio::epsilon && new_offset == 0) {
        throw std::invalid_argument("could not parse float");
    }

    return {ret, new_offset};
}

StringToNumber& getStringToNumber() {
    static StringToNumber stn;
    return stn;
}

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#undef freelocale
#undef strtol_l
#endif

#undef strto_float

}  // namespace morphio
