#include <iostream>

static bool almost_equal(morphio::floatType a, double expected, double epsilon) {
#ifdef MORPHIO_USE_DOUBLE
    bool res = std::abs(a - expected) < epsilon;
#else
    bool res = std::abs(static_cast<double>(a) - expected) < epsilon;
#endif
    if (!res) {
        std::cerr << "Failed almost equal: " << a << " != " << expected
                  << " (expected) with epsilon of " << epsilon << '\n';
    }
    return res;
}

static bool array_almost_equal(const std::vector<morphio::floatType>& a,
                               const std::vector<double>& expected,
                               double epsilon) {
    if (a.size() != expected.size()) {
        return false;
    }
    for (size_t i = 0; i < a.size(); i++) {
        if (!almost_equal(a.at(i), expected.at(i), epsilon)) {
            return false;
        }
    }
    return true;
}
