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
    if(ret == 0 && pos == *endpos) {
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

int64_t StringToNumber::toInt(const char* pos, const char** endpos, int base) const {
    return strtol_l(pos, const_cast<char**>(endpos), base, locale);
}

floatType StringToNumber::toFloat(const char* pos, const char** endpos) const {
    floatType ret = strtof_l(pos, const_cast<char**>(endpos), locale);
    if(ret == 0 && pos == *endpos) {
        throw std::invalid_argument("could not parse float");
    }
    return ret;
}
#endif

StringToNumber& getStringToNumber() {
    static StringToNumber stn;
    return stn;
}
}  // namespace morphio
