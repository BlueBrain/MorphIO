/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <morphio/errorMessages.h>
#include <morphio/mut/mitochondria.h>
#include <morphio/mut/morphology.h>
#include <morphio/mut/section.h>
#include <morphio/mut/writers.h>
#include <morphio/version.h>

#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>
#include <highfive/H5Object.hpp>

#include "../shared_utils.hpp"
#include "writer_utils.h"

namespace {

/**
   A structure to get the base type of nested vectors
   https://stackoverflow.com/a/30960730
 **/
template <typename T>
struct base_type {
    using type = T;
};

template <typename T>
struct base_type<std::vector<T>>: base_type<T> {};

template <typename T>
HighFive::Attribute write_attribute(HighFive::File& file,
                                    const std::string& name,
                                    const T& version) {
    HighFive::Attribute a_version =
        file.createAttribute<typename T::value_type>(name, HighFive::DataSpace::From(version));
    a_version.write(version);
    return a_version;
}

template <typename T>
HighFive::Attribute write_attribute(HighFive::Group& group,
                                    const std::string& name,
                                    const T& version) {
    HighFive::Attribute a_version =
        group.createAttribute<typename T::value_type>(name, HighFive::DataSpace::From(version));
    a_version.write(version);
    return a_version;
}

template <typename T>
void write_dataset(HighFive::File& file, const std::string& name, const T& raw) {
    HighFive::DataSet dpoints =
        file.createDataSet<typename base_type<T>::type>(name, HighFive::DataSpace::From(raw));

    dpoints.write(raw);
}

template <typename T>
void write_dataset(HighFive::Group& file, const std::string& name, const T& raw) {
    HighFive::DataSet dpoints =
        file.createDataSet<typename base_type<T>::type>(name, HighFive::DataSpace::From(raw));

    dpoints.write(raw);
}


}  // anonymous namespace

namespace morphio {
namespace mut {
namespace writer {
namespace {

void mitochondriaH5(HighFive::File& h5_file, const Mitochondria& mitochondria) {
    if (mitochondria.rootSections().empty()) {
        return;
    }

    Property::Properties properties;
    mitochondria._buildMitochondria(properties);
    auto& p = properties._mitochondriaPointLevel;
    size_t size = p._diameters.size();

    std::vector<std::vector<morphio::floatType>> points;
    std::vector<std::vector<int32_t>> structure;
    points.reserve(size);
    for (unsigned int i = 0; i < size; ++i) {
        points.push_back({static_cast<morphio::floatType>(p._sectionIds[i]),
                          p._relativePathLengths[i],
                          p._diameters[i]});
    }

    auto& s = properties._mitochondriaSectionLevel;
    structure.reserve(s._sections.size());
    for (const auto& section : s._sections) {
        structure.push_back({section[0], section[1]});
    }

    HighFive::Group g_organelles = h5_file.createGroup("organelles");
    HighFive::Group g_mitochondria = g_organelles.createGroup("mitochondria");

    write_dataset(g_mitochondria, "points", points);
    write_dataset(g_mitochondria, "structure", structure);
}


void endoplasmicReticulumH5(HighFive::File& h5_file, const EndoplasmicReticulum& reticulum) {
    if (reticulum.sectionIndices().empty()) {
        return;
    }

    HighFive::Group g_organelles = h5_file.createGroup("organelles");
    HighFive::Group g_reticulum = g_organelles.createGroup("endoplasmic_reticulum");

    write_dataset(g_reticulum, "section_index", reticulum.sectionIndices());
    write_dataset(g_reticulum, "volume", reticulum.volumes());
    write_dataset(g_reticulum, "filament_count", reticulum.filamentCounts());
    write_dataset(g_reticulum, "surface_area", reticulum.surfaceAreas());
}

void dendriticSpinePostSynapticDensityH5(HighFive::File& h5_file,
                                         const Property::DendriticSpine::Level& l) {
    const auto& psd = l._post_synaptic_density;

    HighFive::Group g_organelles = h5_file.createGroup("organelles");
    HighFive::Group g_postsynaptic_density = g_organelles.createGroup("postsynaptic_density");

    std::vector<morphio::Property::DendriticSpine::SectionId_t> sectionIds;
    sectionIds.reserve(psd.size());
    std::vector<morphio::Property::DendriticSpine::SegmentId_t> segmentIds;
    segmentIds.reserve(psd.size());
    std::vector<morphio::Property::DendriticSpine::Offset_t> offsets;
    offsets.reserve(psd.size());

    for (const auto& v : psd) {
        sectionIds.push_back(v.sectionId);
        segmentIds.push_back(v.segmentId);
        offsets.push_back(v.offset);
    }
    write_dataset(g_postsynaptic_density, "section_id", sectionIds);
    write_dataset(g_postsynaptic_density, "segment_id", segmentIds);
    write_dataset(g_postsynaptic_density, "offset", offsets);
}
}  // anonymous namespace

void h5(const Morphology& morph, const std::string& filename) {
    if (details::emptyMorphology(morph)) {
        return;
    }

    details::validateContourSoma(morph);
    details::checkSomaHasSameNumberPointsDiameters(*morph.soma());

    HighFive::File h5_file(filename,
                           HighFive::File::ReadWrite | HighFive::File::Create |
                               HighFive::File::Truncate);

    std::unordered_map<uint32_t, int32_t> newIds;

    std::vector<std::vector<morphio::floatType>> raw_points;
    std::vector<std::vector<int32_t>> raw_structure;
    std::vector<morphio::floatType> raw_perimeters;

    const std::vector<Point>& somaPoints = morph.soma()->points();
    const auto& somaDiameters = morph.soma()->diameters();

    for (unsigned int i = 0; i < somaPoints.size(); ++i) {
        raw_points.push_back(
            {somaPoints[i][0], somaPoints[i][1], somaPoints[i][2], somaDiameters[i]});

        // If the morphology has some perimeter data, we need to fill some
        // perimeter dummy value in the soma range of the data structure to keep
        // the length matching
        if (details::hasPerimeterData(morph)) {
            raw_perimeters.push_back(0);
        }
    }

    raw_structure.push_back({0, SECTION_SOMA, -1});
    size_t offset = morph.soma()->points().size();

    int sectionIdOnDisk = 1;
    for (auto it = morph.depth_begin(); it != morph.depth_end(); ++it) {
        const std::shared_ptr<Section>& section = *it;

        const auto& points = section->points();
        const auto& diameters = section->diameters();
        const auto& perimeters = section->perimeters();

        int parentOnDisk = (section->isRoot() ? 0 : newIds[section->parent()->id()]);
        raw_structure.push_back({static_cast<int>(offset), section->type(), parentOnDisk});

        const auto numberOfPoints = points.size();
        for (unsigned int i = 0; i < numberOfPoints; ++i) {
            raw_points.push_back({points[i][0], points[i][1], points[i][2], diameters[i]});
        }

        const auto numberOfPerimeters = perimeters.size();
        if (numberOfPerimeters > 0) {
            if (numberOfPerimeters != numberOfPoints) {
                throw WriterError(readers::ErrorMessages().ERROR_VECTOR_LENGTH_MISMATCH(
                    "points", numberOfPoints, "perimeters", numberOfPerimeters));
            }
            for (unsigned int i = 0; i < numberOfPerimeters; ++i) {
                raw_perimeters.push_back(perimeters[i]);
            }
        }

        newIds[section->id()] = sectionIdOnDisk++;
        offset += numberOfPoints;
    }

    write_dataset(h5_file, "/points", raw_points);
    write_dataset(h5_file, "/structure", raw_structure);

    HighFive::Group g_metadata = h5_file.createGroup("metadata");

    write_attribute(g_metadata, "version", std::array<uint32_t, 2>{1, 3});
    write_attribute(g_metadata,
                    "cell_family",
                    std::vector<uint32_t>{static_cast<uint32_t>(morph.cellFamily())});
    write_attribute(h5_file, "comment", std::vector<std::string>{details::version_string()});

    if (details::hasPerimeterData(morph)) {
        write_dataset(h5_file, "/perimeters", raw_perimeters);
    }

    mitochondriaH5(h5_file, morph.mitochondria());
    endoplasmicReticulumH5(h5_file, morph.endoplasmicReticulum());
    if (morph.cellFamily() == SPINE) {
        dendriticSpinePostSynapticDensityH5(h5_file, morph._dendriticSpineLevel);
    }
}

}  // end namespace writer
}  // end namespace mut
}  // end namespace morphio
