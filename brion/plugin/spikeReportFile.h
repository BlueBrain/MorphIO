/* Copyright (c) 2013-2017, EPFL/Blue Brain Project
 *                          Raphael Dumusc <raphael.dumusc@epfl.ch>
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

#ifndef BRION_PLUGIN_SPIKEREPORTFILE_H
#define BRION_PLUGIN_SPIKEREPORTFILE_H

#include <brion/types.h>
#include "spikeReportTypes.h"

#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>

namespace brion
{
namespace plugin
{

/** A reader for spike report files.
 *
 * A spike report is a text file format in which each line contains a pair of
 * spike time and cell GID values separated by one or more whitespace. Spike
 * times are floating point values in miliseconds units. Cell GIDs are unsigned
 * integer values which uniquely identify a cell within the current experiment.
 * Lines beginning with a '/' or '#' are treated as comments and ignored by the
 * parser. An example file could look like those below:
 *
 * Bluron report format :
 * <BLOCKQUOTE>
 * /scatter<BR>
 * 2.5     1<BR>
 * 6.6     129<BR>
 * 9.8     257<BR>
 * 9.825   385<BR>
 * </BLOCKQUOTE>
 *
 * NEST report format :
 * <BLOCKQUOTE>
 * 1       2.5<BR>
 * 129     6.6<BR>
 * 257     9.8<BR>
 * 385     9.825<BR>
 * </BLOCKQUOTE>
 */
class SpikeReportFile : public boost::noncopyable
{
public:
    /**
     * Open a new report file.
     * @param filename The path to the report file.
     * @param rt The Type of report.
     * @throws std::runtime_error if the file could not be opened.
     */
    SpikeReportFile( const std::string& filename, const SpikeReportType rt,
                     const int accessMode );
    virtual ~SpikeReportFile();

    /**
     * Read the file, appending results to the given Spikes.
     *
     * If the maximum line count is reached before the end of file the next
     * call will resume loading from the last line read.
     * @param spikes The object to store the file contents.
     * @param lineCount The maximum number of content lines to read
     *        from the file. Content lines are non comment or empty lines.
     * @return true is the end of file was reached, false otherwise
     * @throws std::runtime_error if an error occured during parsing or file
     *         is opened for writing.
     */
    bool fillReportMap( SpikeMap& spikes,
                        const size_t maxLines =
                            std::numeric_limits< size_t >::max( ));

    /**
     * Write to the file.
     *
     * @throws std::runtime_error if file is not opened for writing.
     */
    void writeReportMap( const Spikes& spikes );

    /** Close file. */
    void close();

private:
    // Returns true if parsing succeeded
    typedef boost::function<
        bool( const std::string&, Spike& ) > SpikeParseFunc;

    typedef boost::function<
        void( std::fstream& ,const Spike& ) > SpikeWriteFunc;

    std::string _filename;
    SpikeReportType _reportType;
    boost::scoped_ptr< std::fstream > _file;
    SpikeParseFunc _spikeParseFunction;
    SpikeWriteFunc _spikeWriteFunction;
};

}
}

#endif
