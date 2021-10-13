#include "contrib/catch.hpp"
#include <morphio/vasc/section.h>
#include <morphio/vasc/vasculature.h>


using Vasculature = morphio::vasculature::Vasculature;

namespace {

struct Files {
    std::string vasculature;

    Files()
        : vasculature("data/h5/vasculature1.h5") {}
};

}  // anonymous namespace


TEST_CASE("vasculature_section_offsets", "[vasculature]") {
    Files files;
    morphio::vasculature::Vasculature morph(files.vasculature);

    const auto& sections = morph.sections();
    size_t n_offsets = sections.size() + 1;

    std::vector<uint32_t> expected_section_offsets;
    expected_section_offsets.reserve(n_offsets);

    uint32_t offset = 0;
    expected_section_offsets.push_back(offset);

    for (const auto& section : sections) {
        offset += static_cast<uint32_t>(section.points().size());
        expected_section_offsets.push_back(offset);
    }

    REQUIRE(morph.sectionOffsets() == expected_section_offsets);
}
