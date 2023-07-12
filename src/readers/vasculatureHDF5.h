/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <memory>  // std::unique_ptr
#include <string>  // std::string
#include <vector>  // std::vector

#include <morphio/errorMessages.h>
#include <morphio/types.h>
#include <morphio/vasc/properties.h>

#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>

namespace morphio {
namespace readers {
namespace h5 {

class VasculatureHDF5
{
  public:
    explicit VasculatureHDF5(const std::string& uri)
        : _err(uri)
        , _uri(uri) {}

    virtual ~VasculatureHDF5() = default;

    vasculature::property::Properties load();

  private:
    void _readDatasets();
    void _readPoints();
    void _readSections();
    void _readSectionTypes();
    void _readConnectivity();

    std::unique_ptr<HighFive::File> _file;

    std::unique_ptr<HighFive::DataSet> _points;
    std::vector<size_t> _pointsDims;

    std::unique_ptr<HighFive::DataSet> _sections;
    std::vector<size_t> _sectionsDims;

    std::unique_ptr<HighFive::DataSet> _connectivity;
    std::vector<size_t> _conDims;

    vasculature::property::Properties _properties;
    bool _write;
    ErrorMessages _err;
    std::string _uri;
};
}  // namespace h5
}  // namespace readers
}  // namespace morphio
