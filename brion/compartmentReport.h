/* Copyright (c) 2013-2017, EPFL/Blue Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
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

#ifndef BRION_COMPARTMENTREPORT
#define BRION_COMPARTMENTREPORT

#include <brion/api.h>
#include <brion/types.h>

#include <boost/noncopyable.hpp>

#include <future>

namespace brion
{
namespace detail
{
class CompartmentReport;
}

/** Read & write access to a CompartmentReport.
 *
 * The supported types are binary (r), HDF5 (rw) and stream (rw) reports.
 *
 * Following RAII, this class is ready to use after the creation and will ensure
 * release of resources upon destruction.
 */
class CompartmentReport : public boost::noncopyable
{
public:
    /** Close compartment report. @version 1.0 */
    BRION_API ~CompartmentReport();

    /** Open given URI to a compartment report for reading and/or writing.
     *
     * @param uri URI to compartment report. The report type is deduced from
     *        here.
     * @param mode the brion::AccessMode bitmask
     * @param gids the neurons of interest in READ_MODE
     * @throw std::runtime_error if compartment report could be opened for read
     *                           or write, cannot be overwritten or it is not
     *                           valid
     * @version 1.0
     */
    BRION_API CompartmentReport(const URI& uri, int mode,
                                const GIDSet& gids = GIDSet());

    /** @return the descriptions of all loaded report backends. @version 1.0 */
    BRION_API static std::string getDescriptions();

    /** @name Read API */
    //@{
    /** Update compartment mapping wrt the given GIDs.
     *
     * Allows to change the GIDs in an open report without throwing away the
     * already opened data, i.e., avoiding the overhead of always creating a new
     * compartment report object when the desired GID set changes. An empty gids
     * set loads all data from the report.
     *
     * @param gids the neurons of interest
     * @version 1.0
     */
    BRION_API void updateMapping(const GIDSet& gids);

    /** @return the current considered GIDs. @version 1.0 */
    BRION_API const GIDSet& getGIDs() const;

    /**
     * @return the index of the given gid.
     * @version 1.0
     * @throw std::runtime_error if the gid is not mapped
     */
    BRION_API size_t getIndex(const uint32_t gid) const;

    /** Get the current mapping of each section of each neuron in each
     *  simulation frame buffer.
     *
     * For instance, getOffsets()[1][15] retrieves the lookup index for the
     * frame buffer for section 15 of neuron with index 1. The neuron index is
     * derived from the order in the GID set provided by updateMapping().
     *
     * @return the offset for each section for each neuron
     * @version 1.0
     */
    BRION_API const SectionOffsets& getOffsets() const;

    /** Get the number of compartments for each section of each neuron provided
     *  by the GID set via updateMapping().
     *
     * The neuron index is derived from the order in the GID set provided by
     * updateMapping().
     *
     * @return the compartment counts for each section for each neuron
     * @version 1.0
     */
    BRION_API const CompartmentCounts& getCompartmentCounts() const;

    /** Get the number of compartments for the given neuron.
     *
     * The neuron index is derived from the order in the GID set provided by
     * updateMapping().
     *
     * @param index neuron index per current GID set
     * @return number of compartments for the given neuron
     * @version 1.0
     */
    BRION_API size_t getNumCompartments(size_t index) const;

    /** @return the current start time of the report. @version 1.0 */
    BRION_API double getStartTime() const;

    /** @return the current end time of the report. @version 1.0 */
    BRION_API double getEndTime() const;

    /** @return the sampling time interval of the report. @version 1.0 */
    BRION_API double getTimestep() const;

    /** @return the data unit of the report. @version 1.0 */
    BRION_API const std::string& getDataUnit() const;

    /** @return the time unit of the report. @version 1.0 */
    BRION_API const std::string& getTimeUnit() const;

    /** @return the number of values of a loaded report frame. @version 1.0 */
    BRION_API size_t getFrameSize() const;

    /** Load report values at the given time stamp.
     *
     * @param timestamp the time stamp of interest
     * @return the report values if found at timestamp, nullptr otherwise
     * @note Until the completion of this operation, the report shouldn't be
     * modified
     * @version 2.0
     */
    BRION_API std::future<floatsPtr> loadFrame(double timestamp) const;

    /** Load all frames inside a given time window.
     *
     * The result will be empty if the window is invalid or falls outside the
     * report window.
     * @param start close left side of the time interval
     * @param end open right side of time interval
     * @return a future with the frames overlapped by the time window
     *         [start, end) and the start timestamps of these frames.
     * @version 2.0
     */
    BRION_API std::future<Frames> loadFrames(double start, double end) const;

    /**
     * @param gid the neuron report to be loaded.
     * @return the number of values of the given neuron report.
     * @throw std::runtime_error if gid is not mapped.
     * @version 1.0
     */
    BRION_API size_t getNeuronSize(uint32_t gid) const;

    /** Load report values for the given neuron.
     *
     * May not be implemented by all backends (e.g. HDF5).
     *
     * @param gid the neuron identifier
     * @return the report values if neuron is found, nullptr otherwise
     * @note Until the completion of this operation, the report shouldn't be
     * modified
     * @version 2.0
     */
    BRION_API std::future<floatsPtr> loadNeuron(uint32_t gid) const;

    /** Set the size of the stream buffer for loaded frames.
     *
     * Configures the number of simulation frame buffers for stream readers.
     * When the consumer is slower than the producer, the producer will block
     * once these buffers are exhausted, otherwise the consumer will block on
     * data availability. A minimum of 1 frame is buffered.
     *
     * @param size the new size of the frame buffer.
     * @version 1.0
     */
    BRION_API void setBufferSize(const size_t size);

    /** @return the number of the simulation frame buffers. @version 1.0 */
    BRION_API size_t getBufferSize() const;

    /** Clears all buffered frames to free memory. @version 1.0 */
    BRION_API void clearBuffer();
    //@}

    /** @name Write API */
    //@{
    /** Write the header information of this report.
     *
     * @param startTime the start time of the report
     * @param endTime the end time of the report
     * @param timestep the timestep between report frames
     * @param dunit the unit of the data, e.g. mV
     * @param tunit the unit of the time, e.g. ms
     * @throw std::invalid_argument if any passed argument is invalid
     * @version 1.0
     */
    BRION_API void writeHeader(double startTime, double endTime,
                               double timestep, const std::string& dunit,
                               const std::string& tunit);

    /** Write the compartment counts for each section for one cell.
     *
     * This should only be called after writeHeader().
     *
     * @param gid the GID of the cell
     * @param counts the number of compartments per section
     * @return false if saving was not successful, true otherwise
     * @version 1.0
     */
    BRION_API bool writeCompartments(uint32_t gid, const uint16_ts& counts);

    /** Write the values for one cell at a point in time.
     *
     * This should only be called after all the required mapping has been saved
     * before.
     *
     * @param gid the GID of the cell
     * @param values the values per compartment to save
     * @param timestamp the timestamp in ms where for this values
     * @return false if saving was not successful, true otherwise
     * @version 1.0
     */
    bool writeFrame(uint32_t gid, const floats& values, double timestamp)
    {
        return writeFrame(gid, values.data(), values.size(), timestamp);
    }
    BRION_API bool writeFrame(uint32_t gid, const float* values, size_t size,
                              double timestamp);

    /** Flush data to output. @return true on success. @version 1.0 */
    BRION_API bool flush();

    /**
     * Remove all data of the report from storage.
     *
     * May not be implemented by all backends, such as file-based reports.
     * @return true if data was removed.
     * @version 1.0
     */
    BRION_API bool erase();
    //@}

private:
    detail::CompartmentReport* _impl;
};
}

#endif
