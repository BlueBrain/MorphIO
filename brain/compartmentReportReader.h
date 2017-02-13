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
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#pragma once

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
    float startTime = 0;

    /** The  end time of the report */
    float endTime = 0;

    /** The sampling time interval of the report*/
    float timeStep = 0;

    /** The  time unit of the report */
    std::string timeUnit;

    /** The  data unit of the report */
    std::string dataUnit;
};

/**
 * Reader for compartment reports.
 *
 * \ifnot pybind
 * Following RAII, all readers are ready for use after the creation and will
 * ensure release of resources upon destruction.
 * \endif
 */
class CompartmentReportReader
{
public:
    /**
     * Open a report in read mode.
     * @param uri URI to compartment report
     * @throw std::runtime_error if compartment report could be opened
     * for read or it is not valid.
     * @version 2.0
     */
    BRAIN_API CompartmentReportReader(const URI& uri);
    BRAIN_API ~CompartmentReportReader();

    /** @return the metadata of the report */
    BRAIN_API const CompartmentReportMetaData& getMetaData() const;

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
    CompartmentReportReader() = delete;
    CompartmentReportReader(const CompartmentReportReader &) = delete;
    CompartmentReportReader(CompartmentReportReader &&) = delete;
    CompartmentReportReader& operator =(const CompartmentReportReader &) = delete;
    CompartmentReportReader& operator =(CompartmentReportReader &&) = delete;


    std::shared_ptr<detail::CompartmentReportReader> _impl;
};

/**
 * Compartment report view.
 *
 * A report view provides access to the data of a subset of neurons from
 * a report.
 */
class CompartmentReportView
{
public:
    BRAIN_API CompartmentReportView(CompartmentReportView&&);
    BRAIN_API CompartmentReportView& operator=(CompartmentReportView&&);
    BRAIN_API ~CompartmentReportView();

    /** @return the parent reader of the view */
    BRAIN_API CompartmentReportReader& getReader() const;

    /** @return the considered GIDs
     * @version 2.0
     */
    BRAIN_API const brion::GIDSet& getGIDs() const;

    /** @return the data mapping of the view.
     * @version 2.0
     */
    BRAIN_API const CompartmentReportMapping& getMapping() const;

    /** Load a frame at the given time stamp.
     *
     * @param timestamp the time stamp of interest
     * @return a frame containing the data if found at timestamp, an empty frame
     * otherwise
     * @version 2.0
     */
    BRAIN_API std::future<CompartmentReportFrame> load(float timestamp);

    /** Load frames between start and end time stamps.
     *
     * @param start the start time stamp
     * @param end the end time stamp
     * @return the frames overlapped by the given time window. The start time
     *         doesn't need to be aligned with the report timestep and the time
     *         interval is open on the right. The result may be empty is the time
     *         window falls out of the report window.
     * @throw std::logic_error if invalid interval
     * @version 2.0
     */
    BRAIN_API std::future<CompartmentReportFrames> load(float start, float end);

    /** Load all the frames.
     * This is equivalent to call load(starTime, endTime)
     * @version 2.0
     */
    BRAIN_API std::future<CompartmentReportFrames> loadAll();

private:
    CompartmentReportView(
        const std::shared_ptr<detail::CompartmentReportReader>&,
        const brion::GIDSet& gids);
    std::unique_ptr<detail::CompartmentReportView> _impl;
    friend class CompartmentReportReader;
};

/**
 * A simulation data frame.
 *
 * A frame contains the values of a scalar variable on compartments at a given
 * timestamp.
 */
class CompartmentReportFrame
{
public:
    BRAIN_API CompartmentReportFrame();
    BRAIN_API ~CompartmentReportFrame();

    BRAIN_API CompartmentReportFrame(CompartmentReportFrame&&) noexcept;
    BRAIN_API CompartmentReportFrame& operator=(CompartmentReportFrame&&) noexcept;

    /** @return the timestamp of the frame.
     * @version 2.0
     */
    BRAIN_API float getTimestamp() const;

    /** @return true if the frame is empty
     * @version 2.0
     */
    BRAIN_API bool empty() const;

    /** @return the data of the frame.
     * The data layout is specified by a CompartmentReportMapping.
     */
    BRAIN_API const floats& getData() const;

private:
    std::unique_ptr<detail::CompartmentReportFrame> _impl;
    friend class CompartmentReportView;
};

/**
 * Data mapping of a compertment report frame.
 *
 * The mapping provides the number of compartments per neuron/section pair and
 * the start offset of each neuron/section pair within a data frame. Mappings
 * are view specific. The indices used to access the information for a neuron
 * are *not* GIDs, but refer to the iteration order of the GIDSet from the view
 * that provides tha mapping.
 */
class CompartmentReportMapping
{
public:
    struct IndexEntry
    {
        // To ensure proper alignment and compactness the order of this fields
        // mustn't be changed.
        uint64_t offset;
        uint32_t gid;
        uint16_t section;
        uint16_t compartmentCount;
    };
    using Index = std::vector<IndexEntry>;

    /**
     * @return return the index of the all the neurons in the view.
     * @version 2.0
     */
    BRAIN_API const Index& getIndex() const;

    /** Get the current mapping of each section of each neuron in each
     * simulation frame buffer.
     * For instance, getOffsets()[1][15] retrieves the lookup index for the
     * frame buffer for section 15 of neuron with index 1. The neuron index is
     * derived from the order in the GID set provided in the view constructor.
     *
     * @return the offset for each section for each neuron
     * @version 2.0
     */
    BRAIN_API const SectionOffsets& getOffsets() const;

    /** Get the number of compartments for each section of each neuron in the
     * view
     * @return the compartment counts for each section for each neuron
     * @version 2.0
     */
    BRAIN_API const CompartmentCounts& getCompartmentCounts() const;

    /** Get the number of compartments for the given neuron.
     * @param index neuron index per current GID set
     * @return number of compartments for the given neuron
     * @version 2.0
     */
    BRAIN_API size_t getNumCompartments(size_t index) const;

private:
    CompartmentReportMapping() = delete;
    CompartmentReportMapping(const CompartmentReportMapping &) = delete;
    CompartmentReportMapping(CompartmentReportMapping &&) = delete;
    CompartmentReportMapping& operator =(const CompartmentReportMapping &) = delete;
    CompartmentReportMapping& operator =(CompartmentReportMapping &&) = delete;
    CompartmentReportMapping(detail::CompartmentReportView*);
    detail::CompartmentReportView* _viewImpl;
    friend struct detail::CompartmentReportView;
};
}
