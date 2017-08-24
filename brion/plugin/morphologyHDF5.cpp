/* Copyright (c) 2013-2017, EPFL/Blue Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                          Juan Hernando <jhernando@fi.upm.es>
 *
 * This file is part of Brion <https://github.com/BlueBrain/Brion>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "morphologyHDF5.h"

#include "../detail/lockHDF5.h"
#include "../detail/morphologyHDF5.h"
#include "../detail/silenceHDF5.h"
#include "../detail/utilsHDF5.h"

#include <brion/version.h>

#include <lunchbox/debug.h>
#include <lunchbox/pluginRegisterer.h>
#include <lunchbox/scopedMutex.h>

#include <H5Cpp.h>

namespace brion
{
namespace plugin
{
namespace
{
lunchbox::PluginRegisterer<MorphologyHDF5> registerer;

struct Loader
{
    MorphologyPlugin& morphology;
    MorphologyInitData& initData;

    H5::H5File file;

    H5::DataSet pointDS;
    hsize_t pointsDims[2];

    H5::DataSet sectionsDS;
    hsize_t sectionsDims[2];

    std::string stage;

    Loader(MorphologyPlugin& m)
        : morphology(m)
        , initData(m.getInitData())
        , stage("repaired")
    {
        lunchbox::ScopedWrite mutex(detail::hdf5Lock());
        detail::SilenceHDF5 silence;

        initData.version = MORPHOLOGY_VERSION_H5_1;
        const std::string path = initData.getURI().getPath();
        try
        {
            file.openFile(path, H5F_ACC_RDONLY);
        }
        catch (const H5::Exception& exc)
        {
            LBTHROW(std::runtime_error("Could not open morphology file " +
                                       path + ": " + exc.getDetailMsg()));
        }

        if (!readV11Metadata() && !readV2Metadata())
        {
            try
            {
                resolveV1();
                initData.version = MORPHOLOGY_VERSION_H5_1;
            }
            catch (...)
            {
                LBTHROW(std::runtime_error("Unknown morphology file format"));
            }
        }
        selectRepairStage();
        readPoints();
        readSections();
        readSectionTypes();
        readPerimeters();
    }

    ~Loader()
    {
        lunchbox::ScopedWrite mutex(detail::hdf5Lock());

        if (pointDS.getId())
            pointDS.close();

        if (sectionsDS.getId())
            sectionsDS.close();

        if (file.getId())
            file.close();
    }

    bool readV11Metadata()
    {
        try
        {
            const H5::Group& metadata = file.openGroup(_g_metadata);
            const H5::Attribute& attr = metadata.openAttribute(_a_version);

            uint32_t version[2];
            attr.read(H5::PredType::STD_U32LE, &version[0]);
            if (version[0] != 1 || version[1] != 1)
                return false;

            initData.version = MORPHOLOGY_VERSION_H5_1_1;

            const H5::Attribute& familyAttr = metadata.openAttribute(_a_family);
            H5::EnumType familyEnum = metadata.openEnumType(_e_family);

            familyAttr.read(familyEnum, &initData.family);

            resolveV1();
            return true;
        }
        catch (const H5::Exception&)
        {
            return false;
        }
    }

    bool readV2Metadata()
    {
        try
        {
            file.openGroup(_g_root);
            initData.version = MORPHOLOGY_VERSION_H5_2;
            return true;
        }
        catch (const H5::Exception&)
        {
            return false;
        }
    }

    void resolveV1()
    {
        pointDS = file.openDataSet("/" + _d_points);
        const H5::DataSpace pointsSpace = pointDS.getSpace();
        if (pointsSpace.getSimpleExtentNdims() != 2 ||
            pointsSpace.getSimpleExtentDims(pointsDims) < 0 ||
            pointsDims[1] != _pointColumns)
        {
            LBTHROW(std::runtime_error("Opening morphology file '" +
                                       file.getFileName() +
                                       "': bad number of dimensions in"
                                       " 'points' dataspace"));
        }

        sectionsDS = file.openDataSet(_d_structure);
        const H5::DataSpace sectionsSpace = sectionsDS.getSpace();
        if (sectionsSpace.getSimpleExtentNdims() != 2 ||
            sectionsSpace.getSimpleExtentDims(sectionsDims) < 0 ||
            sectionsDims[1] != _structureV1Columns)
        {
            LBTHROW(std::runtime_error("Opening morphology file '" +
                                       file.getFileName() +
                                       "': bad number of dimensions in"
                                       " 'structure' dataspace"));
        }
    }

    void selectRepairStage()
    {
        if (initData.version != MORPHOLOGY_VERSION_H5_2)
            return;

        Strings stages{"repaired", "unraveled", "raw"};
        for (const auto& candidate : stages)
        {
            try
            {
                file.openDataSet("/" + _g_root + "/" + candidate + "/" +
                                 _d_points);
                stage = candidate;
                break;
            }
            catch (const H5::Exception&)
            {
            }
        }
        stage = "repaired";
    }

    void readPoints()
    {
        auto& points = morphology.getPoints();

        if (initData.version == MORPHOLOGY_VERSION_H5_2)
        {
            H5::DataSet dataset;
            try
            {
                dataset = file.openDataSet("/" + _g_root + "/" + stage + "/" +
                                           _d_points);
            }
            catch (...)
            {
                LBERROR << "Could not open points dataset for morphology file "
                        << file.getFileName() << " repair stage " << stage
                        << std::endl;
                return;
            }

            hsize_t dims[2];
            const H5::DataSpace& dspace = dataset.getSpace();
            if (dspace.getSimpleExtentNdims() != 2 ||
                dspace.getSimpleExtentDims(dims) < 0 ||
                dims[1] != _pointColumns)
            {
                LBTHROW(std::runtime_error("Reading morphology file '" +
                                           file.getFileName() +
                                           "': bad number of dimensions in"
                                           " 'points' dataspace"));
            }

            points.resize(dims[0]);
            dataset.read(points.data(), H5::PredType::NATIVE_FLOAT);
            return;
        }

        points.resize(pointsDims[0]);
        pointDS.read(points.data(), H5::PredType::NATIVE_FLOAT);
    }

    void readSections()
    {
        auto& sections = morphology.getSections();

        if (initData.version == MORPHOLOGY_VERSION_H5_2)
        {
            // fixes BBPSDK-295 by restoring old BBPSDK 0.13 implementation
            const std::string stage_(stage == "unraveled" ? "raw" : stage);
            H5::DataSet dataset;
            try
            {
                dataset = file.openDataSet("/" + _g_root + "/" + _g_structure +
                                           "/" + stage_);
            }
            catch (...)
            {
                LBERROR
                    << "Could not open sections dataset for morphology file "
                    << file.getFileName() << " repair stage " << stage_
                    << std::endl;
                return;
            }

            hsize_t dims[2];
            const H5::DataSpace& dspace = dataset.getSpace();
            if (dspace.getSimpleExtentNdims() != 2 ||
                dspace.getSimpleExtentDims(dims) < 0 ||
                dims[1] != _structureV2Columns)
            {
                LBTHROW(std::runtime_error("Reading morphology file '" +
                                           file.getFileName() +
                                           "': bad number of dimensions in"
                                           " 'structure' dataspace"));
            }

            sections.resize(dims[0]);
            dataset.read(sections.data(), H5::PredType::NATIVE_INT);
            return;
        }

        const hsize_t readCount[2] = {sectionsDims[0], 1};
        const hsize_t readOffset[2] = {0, 1};
        H5::DataSpace dspace = sectionsDS.getSpace();
        dspace.selectHyperslab(H5S_SELECT_XOR, readCount, readOffset);

        sections.resize(sectionsDims[0]);
        const hsize_t mdim[2] = {sectionsDims[0], 2};
        const H5::DataSpace mspace(2, mdim);
        sectionsDS.read(sections.data(), H5::PredType::NATIVE_INT, mspace,
                        dspace);
    }

    void readSectionTypes()
    {
        auto& types = morphology.getSectionTypes();

        if (initData.version == MORPHOLOGY_VERSION_H5_2)
        {
            const H5::DataSet& dataset = file.openDataSet(
                "/" + _g_root + "/" + _g_structure + "/" + _d_type);

            hsize_t dims[2];
            const H5::DataSpace& dspace = dataset.getSpace();
            if (dspace.getSimpleExtentNdims() != 2 ||
                dspace.getSimpleExtentDims(dims) < 0 || dims[1] != 1)
            {
                LBTHROW(std::runtime_error("Reading morphology file '" +
                                           file.getFileName() +
                                           "': bad number of dimensions in"
                                           " 'sectiontype' dataspace"));
            }

            types.resize(dims[0]);
            dataset.read(types.data(), H5::PredType::NATIVE_INT);
            return;
        }

        const hsize_t readCount[2] = {sectionsDims[0], 1};
        const hsize_t readOffset[2] = {0, 1};
        H5::DataSpace dspace = sectionsDS.getSpace();
        dspace.selectHyperslab(H5S_SELECT_SET, readCount, readOffset);

        types.resize(sectionsDims[0]);
        const hsize_t mdim = sectionsDims[0];
        const H5::DataSpace mspace(1, &mdim);
        sectionsDS.read(types.data(), H5::PredType::NATIVE_INT, mspace, dspace);
    }

    void readPerimeters()
    {
        auto& perimeters = morphology.getPerimeters();

        if (initData.version != MORPHOLOGY_VERSION_H5_1_1)
            return;

        try
        {
            H5::DataSet dataset = file.openDataSet(_d_perimeters);

            hsize_t dims;
            const H5::DataSpace& dspace = dataset.getSpace();
            if (dspace.getSimpleExtentNdims() != 1 ||
                dspace.getSimpleExtentDims(&dims) < 0)
            {
                LBTHROW(std::runtime_error("Reading morphology file '" +
                                           file.getFileName() +
                                           "': bad number of dimensions in"
                                           " 'perimeters' dataspace"));
            }

            perimeters.resize(dims);
            dataset.read(perimeters.data(), H5::PredType::NATIVE_FLOAT);
        }
        catch (...)
        {
            if (initData.family == FAMILY_GLIA)
                LBTHROW(
                    std::runtime_error("No empty perimeters allowed for glia "
                                       "morphology"));
        }
    }
};
}

MorphologyHDF5::MorphologyHDF5(const MorphologyInitData& initData)
    : MorphologyPlugin(initData)
{
}

void MorphologyHDF5::load()
{
    Loader loader(*this);
}

MorphologyHDF5::~MorphologyHDF5()
{
}

bool MorphologyHDF5::handles(const MorphologyInitData& initData)
{
    const std::string& scheme = initData.getURI().getScheme();
    if (scheme != "file" && !scheme.empty())
        return false;

    const std::string& path = initData.getURI().getPath();
    const size_t pos = path.find_last_of(".");
    if (pos == std::string::npos)
        return false;

    return path.substr(pos) == ".h5";
}

std::string MorphologyHDF5::getDescription()
{
    return "Blue Brain hdf5 morphologies:\n"
           "  [file://]/path/to/morphology.h5";
}
}
}
