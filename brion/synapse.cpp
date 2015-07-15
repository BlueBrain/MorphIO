/* Copyright (c) 2013-2015, EPFL/Blue Brain Project
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

#include "synapse.h"
#include "detail/lockHDF5.h"
#include "detail/silenceHDF5.h"

#include <bitset>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <H5Cpp.h>
#include <lunchbox/log.h>
#include <lunchbox/scopedMutex.h>

namespace brion
{
namespace detail
{

struct Dataset
{
    H5::DataSet dataset;
    H5::DataSpace dataspace;
    hsize_t dims[2];
};

class Synapse : public boost::noncopyable
{
public:
    explicit Synapse( const std::string& source )
    {
        lunchbox::ScopedWrite mutex( detail::_hdf5Lock );

        try
        {
            SilenceHDF5 silence;
            _file.openFile( source, H5F_ACC_RDONLY );
        }
        catch( const H5::Exception& exc )
        {
            LBTHROW( std::runtime_error( "Could not open synapse file " + source
                                         + ": " + exc.getDetailMsg( )));
        }

        Dataset dataset;
        const std::string& datasetName = _file.getObjnameByIdx( 0 );
        if( !_loadDataset( datasetName, dataset ))
            LBTHROW( std::runtime_error( "Cannot open dataset in synapse file "
                                         + source ));

        _numAttributes = dataset.dims[1];
        if( _numAttributes != SYNAPSE_ALL &&
            _numAttributes != SYNAPSE_POSITION_ALL )
        {
            LBTHROW( std::runtime_error( source + " not a valid synapse file"));
        }
    }

    ~Synapse()
    {
        lunchbox::ScopedWrite mutex( detail::_hdf5Lock );

        _file.close();
    }

    template< size_t N >
    SynapseMatrix read( const uint32_t gid, const uint32_t attributes ) const
    {
        const std::bitset< N > bits( attributes );
        if( !bits.any( ))
            return SynapseMatrix();

        lunchbox::ScopedWrite mutex( detail::_hdf5Lock );

        Dataset dataset;
        if( !_loadDataset( gid, dataset ))
            return SynapseMatrix();

        dataset.dataspace.selectNone();
        for( size_t i = 0; i < bits.size(); ++i )
        {
            if( bits.test( i ))
            {
                const hsize_t readCounts[2] = { dataset.dims[0], 1 };
                const hsize_t readOffsets[2] = { 0, i };
                dataset.dataspace.selectHyperslab( H5S_SELECT_OR, readCounts,
                                                   readOffsets );
            }
        }

        SynapseMatrix values( boost::extents[dataset.dims[0]][bits.count()] );
        const hsize_t targetSizes[2] = { dataset.dims[0], bits.count() };
        H5::DataSpace targetspace( 2, targetSizes );

        dataset.dataset.read( values.data(), H5::PredType::NATIVE_FLOAT,
                              targetspace, dataset.dataspace );
        return values;
    }

    size_t getNumSynapses( const GIDSet& gids ) const
    {
        lunchbox::ScopedWrite mutex( detail::_hdf5Lock );

        size_t numSynapses = 0;
        BOOST_FOREACH( const uint32_t gid, gids )
        {
            Dataset dataset;
            if( !_loadDataset( gid, dataset ))
                continue;
            numSynapses += dataset.dims[0];
        }
        return numSynapses;
    }

    bool _loadDataset( const uint32_t gid, Dataset& dataset ) const
    {
        std::stringstream name;
        name << "a" << gid;
        return _loadDataset( name.str(), dataset );
    }

    bool _loadDataset( const std::string& name, Dataset& dataset ) const
    {
        try
        {
            SilenceHDF5 silence;
            dataset.dataset = _file.openDataSet( name );
        }
        catch( const H5::Exception& )
        {
            LBVERB << "Could not find synapse dataset for " << name << ": "
                   << std::endl;
            return false;
        }

        dataset.dataspace = dataset.dataset.getSpace();
        if( dataset.dataspace.getSimpleExtentNdims() != 2 )
        {
            LBERROR << "Synapse dataset is not 2 dimensional" << std::endl;
            return false;
        }

        if( dataset.dataspace.getSimpleExtentDims( dataset.dims ) < 0 )
        {
            LBERROR << "Synapse dataset dimensions could not be retrieved"
                    << std::endl;
            return false;
        }

        return true;
    }

    H5::H5File _file;
    size_t _numAttributes;
};
}

Synapse::Synapse( const std::string& source )
    : _impl( new detail::Synapse( source ))
{
}

Synapse::~Synapse()
{
    delete _impl;
}

SynapseMatrix Synapse::read( const uint32_t gid,
                             const uint32_t attributes ) const
{
    switch( _impl->_numAttributes )
    {
    case SYNAPSE_ALL:
        return _impl->read< SYNAPSE_ALL >( gid, attributes );
    case SYNAPSE_POSITION_ALL:
        return _impl->read< SYNAPSE_POSITION_ALL >( gid, attributes );
    default:
        LBERROR << "Synapse file " << _impl->_file.getFileName()
                << " has unknown number of attributes: "
                << _impl->_numAttributes << std::endl;
        return SynapseMatrix();
    }
}

size_t Synapse::getNumSynapses( const GIDSet& gids ) const
{
    return _impl->getNumSynapses( gids );
}

}
