#include <iostream>

static bool almost_equal(morphio::floatType actual, double expected, double epsilon) {
#ifdef MORPHIO_USE_DOUBLE
    bool res = std::abs(actual - expected) < epsilon;
#else
    bool res = std::abs(static_cast<double>(actual) - expected) < epsilon;
#endif
    if (!res) {
        std::cerr << "Failed almost equal: " << actual << " != " << expected
                  << " (expected) with epsilon of " << epsilon << '\n';
    }
    return res;
}

static bool array_almost_equal(const std::vector<morphio::floatType>& actual,
                               const std::vector<double>& expected,
                               double epsilon) {
    if (actual.size() != expected.size()) {
        return false;
    }
    for (size_t i = 0; i < actual.size(); i++) {
        if (!almost_equal(actual[i], expected[i], epsilon)) {
            return false;
        }
    }
    return true;
}
