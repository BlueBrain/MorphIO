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

namespace brain
{
namespace detail
{
struct CompartmentReportView;
}

/**
 * Data mapping of a compartment report frame.
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
        // To ensure proper alignment and compactness for the python binding the
        // order of these fields mustn't be changed.
        uint32_t gid;
        uint32_t section;
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
     * derived from its positions in the sorted list of GIDs provided in the
     * view constructor.
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

    /** @return The total number of compartments in a frame.
     * @version 3.0
     */
    BRAIN_API size_t getFrameSize() const;

private:
    CompartmentReportMapping() = delete;
    CompartmentReportMapping(const CompartmentReportMapping&) = delete;
    CompartmentReportMapping(CompartmentReportMapping&&) = delete;
    CompartmentReportMapping& operator=(const CompartmentReportMapping&) =
        delete;
    CompartmentReportMapping& operator=(CompartmentReportMapping&&) = delete;
    CompartmentReportMapping(detail::CompartmentReportView*);
    detail::CompartmentReportView* _viewImpl;
    friend struct detail::CompartmentReportView;
};
}
