/* Copyright (c) 2017, EPFL/Blue Brain Project
 *                     Juan Hernando <juan.hernando@epfl.ch>
 *                     Mohamed-Ghaith Kaabi <mohamed.kaabi@epfl.ch>
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
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#pragma once

#include "compartmentReportView.h"

#include <brain/api.h>
#include <brain/types.h>
#include <future>

namespace brain
{
namespace detail
{
struct CompartmentReportReader;
struct CompartmentReportFrame;
struct CompartmentReportView;
}

/**
 * Compartment report meta data.
 */
struct CompartmentReportMetaData
{
    /** The  start time of the report */
    double startTime = 0;

    /** The  end time of the report */
    double endTime = 0;

    /** The sampling time interval of the report*/
    double timeStep = 0;

    /** The  time unit of the report */
    std::string timeUnit;

    /** The  data unit of the report */
    std::string dataUnit;

    /** The cell count of the report */
    size_t cellCount;

    /** The total frame count in the report */
    size_t frameCount;

    /** The total compartment count in the report */
    size_t compartmentCount;
};

/**
 * Reader for compartment reports.
 *
 * \ifnot pybind
 * Following RAII, all readers are ready for use after the creation and will
 * ensure release of resources upon destruction.
 * \endif
 */
class CompartmentReport
{
public:
    /**
     * Open a report in read mode.
     * @param uri URI to compartment report
     * @throw std::runtime_error if compartment report could be opened
     * for read or it is not valid.
     * @version 2.0
     */
    BRAIN_API CompartmentReport(const URI& uri);
    BRAIN_API ~CompartmentReport();

#ifndef DOXYGEN_TO_BREATHE
    /** @return the metadata of the report */
    BRAIN_API const CompartmentReportMetaData& getMetaData() const;
#else
    /** @return the metadata of the report
     * - *start_time* (Numeric) : The  start time of the report
     * - *end_time* (Numeric) : The  end time of the report
     * - *time_step* (Numeric) : The sampling time interval of the report
     * - *time_unit* (String) : The  time unit of the report
     * - *data_unit* (String) : The  data unit of the report
     * - *cell_count* (Numeric) : The cell count of the report
     * - *compartment_count* (Numeric) : The total compartment count in the
     * report
     * - *frame_count* (Numeric) : The total frame count in the report
    */
    dict getMetaData() const;
#endif

    /** @return the GIDs of the report */
    BRAIN_API const brion::GIDSet& getGIDs() const;

    /**
     * Create a view of a subset of neurons. An empty gid
     * set creates a view containing all the data.
     *
     * @param gids the neurons of interest
     * @throw std::runtime_error if invalid GID set.
     * @version 2.0
     */
    BRAIN_API CompartmentReportView createView(const GIDSet& gids);

    /**
     * Create a view with all the neurons in the report.
     *
     * @version 2.0
     */
    BRAIN_API CompartmentReportView createView();

private:
    CompartmentReport() = delete;
    CompartmentReport(const CompartmentReport&) = delete;
    CompartmentReport(CompartmentReport&&) = delete;
    CompartmentReport& operator=(const CompartmentReport&) = delete;
    CompartmentReport& operator=(CompartmentReport&&) = delete;

    std::shared_ptr<detail::CompartmentReportReader> _impl;
};
}
