#include <morphio/errorMessages.h>
#include <morphio/types.h>

#include "point_utils.h"


namespace morphio {

template <typename T, size_t N>
std::string valueToString(const std::array<T, N>& a) {
    std::ostringstream oss;
    std::copy(a.begin(), a.end(), std::ostream_iterator<T>(oss, ", "));
    return oss.str();
}

template <typename T>
std::string valueToString(const T a) {
    return std::to_string(a);
}

template <typename T>
void _appendVector(std::vector<T>& to, const std::vector<T>& from, int offset) {
    to.insert(to.end(), from.begin() + offset, from.end());
}

template <typename T>
std::vector<typename T::Type> copySpan(const std::vector<typename T::Type>& data,
                                       SectionRange range) {
    if (data.empty())
        return {};

    return {data.begin() + static_cast<long int>(range.first),
            data.begin() + static_cast<long int>(range.second)};
}

}  // namespace morphio
