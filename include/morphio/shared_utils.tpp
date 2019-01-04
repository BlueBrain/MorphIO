#include <morphio/errorMessages.h>
#include <morphio/types.h>

namespace morphio {
template <typename ContainerDiameters, typename ContainerPoints>
float _somaSurface(const SomaType type, const ContainerDiameters& diameters,
    const ContainerPoints& points)
{
    size_t size = points.size();
    if (size == 0)
        return 0.;

    switch (type) {
    case SOMA_SINGLE_POINT: {
        float radius = diameters[0] / 2;
        return 4 * M_PI * radius * radius;
    }

    case SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS: {
        float radius = diameters[0] / 2;
        return 4 * M_PI * radius * radius;
    }
    case SOMA_CYLINDERS: {
        // Surface is approximated as the sum of areas of the conical frustums
        // defined by each segments. Does not include the endcaps areas
        float surface = 0;
        for (unsigned int i = 0; i < size - 1; ++i) {
            float r0 = diameters[i] * 0.5;
            float r1 = diameters[i + 1] * 0.5;
            float h2 = distance(points[i], points[i + 1]);
            auto s = M_PI * (r0 + r1) * sqrt((r0 - r1) * (r0 - r1) + h2 * h2);
            surface += s;
        }
        return surface;
    }
    case SOMA_SIMPLE_CONTOUR: {
        throw NotImplementedError(
            "Surface is not implemented for SOMA_SIMPLE_CONTOUR");
    }
    default: {
        morphio::plugin::ErrorMessages err;
        LBTHROW(SomaError(
            err.ERROR_NOT_IMPLEMENTED_UNDEFINED_SOMA("Soma::surface")));
    }
    }
}

} // namespace morphio
