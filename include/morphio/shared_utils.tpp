#include <morphio/types.h>
#include <morphio/errorMessages.h>


namespace morphio
{
template <typename ContainerDiameters, typename ContainerPoints>
const float _somaSurface(const SomaType type,
                         ContainerDiameters diameters,
                         ContainerPoints points){
    switch(type) {
    case SOMA_SINGLE_POINT:
    {
        float radius = diameters[0] / 2;
        return 4 * M_PI * radius * radius;
    }

    case SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS:
    {
        float radius = diameters[0] / 2;
        return 4 * M_PI * radius * radius;
    }
    case SOMA_CYLINDERS:
    {
        // Surface is approximated as the sum of areas of the conical frustums
        // defined by each segments. Does not include the surface areas
        float surface = 0;
        size_t size = points.size();
        for(int i = 0;i<size-1; ++i){
            float r0 = diameters[i];
            float r1 = diameters[i+1];
            float h2 = distance(points[i], points[i+1]);
            surface += M_PI * (r0 + r1) * sqrt((r0 - r1) * (r0 - r1) + h2);
        }
        return surface;
    }
    case SOMA_THREE_POINTS:
    {
        float radius = 0.5 * (distance(points[0], points[1]),
                              distance(points[0], points[2]));
        return 4 * M_PI * radius * radius;
    }
    case SOMA_SIMPLE_CONTOUR:
    {

    }
    case SOMA_UNDEFINED:
    {
        morphio::plugin::ErrorMessages err;
        LBTHROW(SomaError(err.ERROR_NOT_IMPLEMENTED_UNDEFINED_SOMA("Soma::surface")));
    }
    }
}

template <typename ContainerPoints>
const std::vector<Point> blah(const ContainerPoints& points){
    // C++ implementation of lib/hoc/import3d/import3d_sec.hoc
    // Note: the comment on the function says 100, but the code says
    // d = new Vector(101), so I use 101 points
    size_t size = points.size();
    std::vector<float> perim;
    perim.push_back(0);
    float lastPerim = 0;
    for (int i = 0; i < size; ++i){
        int nextIndex = (i == size-1 ? 0 : i+1);
        float dist = distance(points[i], points[nextIndex]);
        lastPerim += dist;
        perim.push_back(lastPerim);
    }

    int nPoints = 101;
    std::vector<morphio::Point> newPoints;
    int j = 0;
    morphio::Point meanPoint;
    for (int i = 0; i < nPoints; ++i) {
        float arc = lastPerim * i / (nPoints - 1);
        while(j < size-1 && perim[j+1] < arc) ++j;
        morphio::Point nextPoint = (j == size-1 ? points[0] : points[j+1]);
        float length_arc_segment = distance(nextPoint, points[j]);
        float fraction = (arc - perim[j]) / length_arc_segment;
        morphio::Point point = points[j] + fraction * (nextPoint - points[j]);
        newPoints.push_back(point);
        meanPoint += point / nPoints;
    }
    return newPoints;
}

} // namespace morphio
