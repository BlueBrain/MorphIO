/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <cstdint>   // uint32_t
#if defined(WIN32) || defined(__WIN32__) || defined(_WIN32) || defined(_MSC_VER) || defined(__MINGW32__)
#define F_OK    0
#include <io.h>
#else
#include <unistd.h>  // access / F_OK
#endif

#include <morphio/vasc/section.h>
#include <morphio/vasc/vasculature.h>

#include "../readers/morphologySWC.h"
#include "../readers/vasculatureHDF5.h"

namespace morphio {
namespace vasculature {

void buildConnectivity(std::shared_ptr<property::Properties> properties);

Vasculature::Vasculature(const std::string& source) {
    const size_t pos = source.find_last_of(".");
    if (pos == std::string::npos) {
        throw UnknownFileType("File has no extension");
    }

    if (access(source.c_str(), F_OK) == -1) {
        throw RawDataError("File: " + source + " does not exist.");
    }

    std::string extension = source.substr(pos);

    property::Properties loader;
    if (extension == ".h5") {
        loader = readers::h5::VasculatureHDF5(source).load();
    } else {
        throw UnknownFileType("File: " + source + " does not end with the .h5 extension");
    }

    properties_ = std::make_shared<property::Properties>(loader);

    buildConnectivity(properties_);
}

Section Vasculature::section(uint32_t id) const {
    return {id, properties_};
}

std::vector<Section> Vasculature::sections() const {
    std::vector<Section> sections_;
    const auto& vasc_sections = properties_->get<property::VascSection>();
    sections_.reserve(vasc_sections.size());
    for (uint32_t i = 0; i < vasc_sections.size(); ++i) {
        sections_.emplace_back(i, properties_);
    }
    return sections_;
}

const std::vector<uint32_t> Vasculature::sectionOffsets() const noexcept {
    // Vasculature section property is a single value representing the offset
    const auto& offsets = properties_->get<property::VascSection>();

    const auto size = offsets.size();
    std::vector<uint32_t> indices(size + 1);

    std::copy(offsets.begin(), offsets.end(), indices.begin());

    indices[size] = static_cast<uint32_t>(this->points().size());

    return indices;
}

const std::vector<morphio::vasculature::property::Connection::Type>&
Vasculature::sectionConnectivity() const noexcept {
    return properties_->get<property::Connection>();
}

graph_iterator Vasculature::begin() const {
    return graph_iterator(*this);
}

graph_iterator Vasculature::end() const {
    return graph_iterator();
}

void buildConnectivity(std::shared_ptr<property::Properties> properties) {
    const std::vector<std::array<unsigned int, 2>>& connectivity =
        properties->get<property::Connection>();
    auto& successors = properties->_sectionLevel._successors;
    auto& predecessors = properties->_sectionLevel._predecessors;

    for (const auto& connection : connectivity) {
        uint32_t first = connection[0];
        uint32_t second = connection[1];
        successors[first].push_back(second);
        predecessors[second].push_back(first);
    }
}

}  // namespace vasculature
}  // namespace morphio
