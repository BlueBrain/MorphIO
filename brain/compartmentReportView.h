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
struct CompartmentReportView;
struct CompartmentReportReader;
}
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
    BRAIN_API CompartmentReport& getReader() const;

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
    BRAIN_API std::future<brion::Frame> load(double timestamp);

    /** Load frames between start and end time stamps.
     *
     * @param start the start time stamp
     * @param end the end time stamp
     * @return the frames overlapped by the given time window. The start time
     *         doesn't need to be aligned with the report timestep and the time
     *         interval is open on the right. The result may be empty if the
     *         time window falls out of the report window.
     * @throw std::logic_error if invalid interval
     * @version 2.0
     */
    BRAIN_API std::future<brion::Frames> load(double start, double end);

    /** Load frames between start and end time stamps.
     *
     * @param start the start time stamp with a time step
     * @param end the end time stamp
     * @param step the time step
     * @return the frames overlapped by the given time window, spaced by a given
     *         step. The start time doesn't need to be aligned with the step
     *         and the time interval is open on the right. The result may be
     *         empty if the time window falls out of the report window.
     * @throw std::logic_error if invalid interval or step < timeStep or step is
     * not a multiple of timeStep
     * @version 2.1
     */
    BRAIN_API std::future<brion::Frames> load(double start, double end,
                                              double step);

    /** Load all the frames.
     * This is equivalent to call load(starTime, endTime)
     * @version 2.0
     */
    BRAIN_API std::future<brion::Frames> loadAll();

private:
    CompartmentReportView(
        const std::shared_ptr<detail::CompartmentReportReader>&,
        const brion::GIDSet& gids);
    std::unique_ptr<detail::CompartmentReportView> _impl;
    friend class CompartmentReport;
};
}
