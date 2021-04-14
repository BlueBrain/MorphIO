#include <morphio/errorMessages.h>
#include <morphio/types.h>


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

template <typename ContainerDiameters, typename ContainerPoints>
floatType _somaSurface(const SomaType type,
                       const ContainerDiameters& diameters,
                       const ContainerPoints& points) {
    size_t size = points.size();

    switch (type) {
    case SOMA_SINGLE_POINT: {
        floatType radius = diameters[0] / 2;
        return 4 * morphio::PI * radius * radius;
    }

    case SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS: {
        floatType radius = diameters[0] / 2;
        return 4 * morphio::PI * radius * radius;
    }
    case SOMA_CYLINDERS: {
        // Surface is approximated as the sum of areas of the conical frustums
        // defined by each segments. Does not include the endcaps areas
        floatType surface = 0;
        for (unsigned int i = 0; i < size - 1; ++i) {
            floatType r0 = static_cast<morphio::floatType>(diameters[i]) * floatType{0.5};
            floatType r1 = static_cast<morphio::floatType>(diameters[i + 1]) * floatType{0.5};
            floatType h2 = distance(points[i], points[i + 1]);
            auto s = morphio::PI * (r0 + r1) * std::sqrt((r0 - r1) * (r0 - r1) + h2 * h2);
            surface += s;
        }
        return surface;
    }
    case SOMA_SIMPLE_CONTOUR: {
        throw NotImplementedError("Surface is not implemented for SOMA_SIMPLE_CONTOUR");
    }
    case SOMA_UNDEFINED:
    default: {
        morphio::readers::ErrorMessages err;
        throw SomaError(err.ERROR_NOT_IMPLEMENTED_UNDEFINED_SOMA("Soma::surface"));
    }
    }
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
