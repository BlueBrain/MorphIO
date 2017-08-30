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
#include "../detail/utilsHDF5.h"

#include <brion/version.h>

#include <lunchbox/debug.h>
#include <lunchbox/pluginRegisterer.h>
#include <lunchbox/scopedMutex.h>

#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>
#include <highfive/H5Utility.hpp>

namespace brion
{
namespace plugin
{
namespace
{
lunchbox::PluginRegisterer<MorphologyHDF5> registerer;

struct Loader
{
    Loader(MorphologyPlugin& m)
        : _morphology(m)
        , _initData(m.getInitData())
        , _stage("repaired")
    {
        lunchbox::ScopedWrite mutex(detail::hdf5Lock());
        const std::string path = _initData.getURI().getPath();

        try
        {
            HighFive::SilenceHDF5 silence;
            _file.reset(new HighFive::File(path, HighFive::File::ReadOnly));
        }
        catch (const HighFive::FileException& exc)
        {
            LBTHROW(std::runtime_error(_write
                                           ? "Could not create morphology file "
                                           : "Could not open morphology file " +
                                                 path + ": " + exc.what()));
        }
        _checkVersion(path);
        _selectRepairStage();
        _readPoints();
        _readSections();
        _readSectionTypes();
        _readPerimeters();
    }

    ~Loader()
    {
        lunchbox::ScopedWrite mutex(detail::hdf5Lock());
        _points.reset();
        _sections.reset();
        _file.reset();
    }

private:
    MorphologyPlugin& _morphology;
    MorphologyInitData& _initData;

    std::unique_ptr<HighFive::File> _file;

    std::unique_ptr<HighFive::DataSet> _points;
    std::vector<size_t> _pointsDims;

    std::unique_ptr<HighFive::DataSet> _sections;
    std::vector<size_t> _sectionsDims;

    std::string _stage;
    bool _write;

    void _checkVersion(const std::string& source)
    {
        if (_readV11Metadata())
            return;

        if (_readV2Metadata())
            return;

        try
        {
            _resolveV1();
            _initData.version = MORPHOLOGY_VERSION_H5_1;
            return;
        }
        catch (...)
        {
            LBTHROW(
                std::runtime_error("Unknown morphology file format for "
                                   "file " +
                                   source));
        }
    }

    void _selectRepairStage()
    {
        if (_initData.version != MORPHOLOGY_VERSION_H5_2)
            return;

        for (const auto& stage : {"repaired", "unraveled", "raw"})
        {
            try
            {
                HighFive::SilenceHDF5 silence;
                _file->getDataSet("/" + _g_root + "/" + stage + "/" +
                                  _d_points);
                _stage = stage;
                break;
            }
            catch (const HighFive::DataSetException&)
            {
            }
        }
        _stage = "repaired";
    }

    void _resolveV1()
    {
        HighFive::SilenceHDF5 silence;
        _points.reset(
            new HighFive::DataSet(_file->getDataSet("/" + _d_points)));
        auto dataspace = _points->getSpace();
        _pointsDims = dataspace.getDimensions();

        if (_pointsDims.size() != 2 || _pointsDims[1] != _pointColumns)
        {
            LBTHROW(std::runtime_error("Opening morphology file '" +
                                       _file->getName() +
                                       "': bad number of dimensions in"
                                       " 'points' dataspace"));
        }

        _sections.reset(new HighFive::DataSet(_file->getDataSet(_d_structure)));
        dataspace = _sections->getSpace();
        _sectionsDims = dataspace.getDimensions();
        if (_sectionsDims.size() != 2 ||
            _sectionsDims[1] != _structureV1Columns)
        {
            LBTHROW(std::runtime_error("Opening morphology file '" +
                                       _file->getName() +
                                       "': bad number of dimensions in"
                                       " 'structure' dataspace"));
        }
    }

    bool _readV11Metadata()
    {
        try
        {
            HighFive::SilenceHDF5 silence;
            const auto metadata = _file->getGroup(_g_metadata);
            const auto attr = metadata.getAttribute(_a_version);

            uint32_t version[2];
            attr.read(version);
            if (version[0] != 1 || version[1] != 1)
                return false;

            _initData.version = MORPHOLOGY_VERSION_H5_1_1;

            const auto familyAttr = metadata.getAttribute(_a_family);
            uint32_t family;
            familyAttr.read(family);
            _initData.family = (CellFamily)family;
        }
        catch (const HighFive::GroupException&)
        {
            return false;
        }
        catch (const HighFive::Exception& e)
        {
            // All other exceptions are not expected because if the metadata
            // group exits it must contain at least the version, and for
            // version 1.1 it must contain the family.
            LBTHROW(std::runtime_error(
                std::string("Error reading morphology metadata: ") + e.what()));
        }

        _resolveV1();
        return true;
    }

    bool _readV2Metadata()
    {
        try
        {
            HighFive::SilenceHDF5 silence;
            const auto root = _file->getGroup(_g_root);
            const auto attr = root.getAttribute(_a_version);
            attr.read(_initData.version);
            if (_initData.version == MORPHOLOGY_VERSION_H5_2)
                return true;
        }
        catch (const HighFive::Exception&)
        {
        }

        try
        {
            HighFive::SilenceHDF5 silence;
            _file->getGroup(_g_root);
            _initData.version = MORPHOLOGY_VERSION_H5_2;
            return true;
        }
        catch (const HighFive::Exception&)
        {
            return false;
        }
    }

    HighFive::DataSet _getStructureDataSet(size_t nSections)
    {
        try
        {
            HighFive::SilenceHDF5 silence;
            return _file->getDataSet(_d_structure);
        }
        catch (const HighFive::DataSetException&)
        {
            return _file->createDataSet<int>(_d_structure, HighFive::DataSpace(
                                                               {nSections, 3}));
        }
    }

    void _readPoints()
    {
        auto& points = _morphology.getPoints();

        if (_initData.version == MORPHOLOGY_VERSION_H5_2)
        {
            auto dataset = [this]() {
                try
                {
                    return _file->getDataSet("/" + _g_root + "/" + _stage +
                                             "/" + _d_points);
                }
                catch (HighFive::DataSetException&)
                {
                    LBTHROW(std::runtime_error(
                        "Could not open points dataset for morphology file " +
                        _file->getName() + " repair stage " + _stage));
                }
            }();

            const auto dims = dataset.getSpace().getDimensions();
            if (dims.size() != 2 || dims[1] != _pointColumns)
            {
                LBTHROW(std::runtime_error(
                    "Reading morphology file '" + _file->getName() +
                    "': bad number of dimensions in 'points' dataspace"));
            }
            points.resize(dims[0]);
            dataset.read(points);
            return;
        }

        points.resize(_pointsDims[0]);
        _points->read(points);
    }

    void _readSections()
    {
        auto& sections = _morphology.getSections();

        if (_initData.version == MORPHOLOGY_VERSION_H5_2)
        {
            // fixes BBPSDK-295 by restoring old BBPSDK 0.13 implementation
            auto dataset = [this]() {
                try
                {
                    return _file->getDataSet("/" + _g_root + "/" +
                                             _g_structure + "/" + _stage);
                }
                catch (HighFive::DataSetException&)
                {
                    LBTHROW(std::runtime_error(
                        "Could not open sections dataset for morphology file " +
                        _file->getName() + " repair stage " + _stage));
                }
            }();

            const auto dims = dataset.getSpace().getDimensions();
            if (dims.size() != 2 || dims[1] != _structureV2Columns)
            {
                LBTHROW(std::runtime_error(
                    "Reading morphology file '" + _file->getName() +
                    "': bad number of dimensions in 'structure' dataspace"));
            }

            sections.resize(dims[0]);
            dataset.read(sections);
            return;
        }

        auto selection =
            _sections->select({0, 0}, {_sectionsDims[0], 2}, {1, 2});
        sections.resize(_sectionsDims[0]);
        selection.read(sections);
    }

    void _readSectionTypes()
    {
        auto& types = _morphology.getSectionTypes();

        if (_initData.version == MORPHOLOGY_VERSION_H5_2)
        {
            auto dataset = [this]() {
                try
                {
                    return _file->getDataSet("/" + _g_root + "/" +
                                             _g_structure + "/" + _d_type);
                }
                catch (HighFive::DataSetException&)
                {
                    LBTHROW(
                        std::runtime_error("Could not open section type "
                                           "dataset for morphology file " +
                                           _file->getName()));
                }
            }();

            const auto dims = dataset.getSpace().getDimensions();
            if (dims.size() != 2 || dims[1] != 1)
            {
                LBTHROW(std::runtime_error(
                    "Reading morphology file '" + _file->getName() +
                    "': bad number of dimensions in 'sectiontype' dataspace"));
            }

            types.resize(dims[0]);
            dataset.read(types);
            return;
        }

        auto selection = _sections->select({0, 1}, {_sectionsDims[0], 1});
        types.resize(_sectionsDims[0]);
        selection.read(types);
    }

    void _readPerimeters()
    {
        if (_initData.version != MORPHOLOGY_VERSION_H5_1_1)
            return;

        try
        {
            HighFive::SilenceHDF5 silence;
            HighFive::DataSet dataset = _file->getDataSet(_d_perimeters);

            auto dims = dataset.getSpace().getDimensions();
            if (dims.size() != 1)
            {
                LBTHROW(std::runtime_error("Reading morphology file '" +
                                           _file->getName() +
                                           "': bad number of dimensions in"
                                           " 'perimeters' dataspace"));
            }

            auto& perimeters = _morphology.getPerimeters();
            perimeters.resize(dims[0]);
            dataset.read(perimeters);
        }
        catch (...)
        {
            if (_initData.family == FAMILY_GLIA)
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
    Loader(*this);
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
