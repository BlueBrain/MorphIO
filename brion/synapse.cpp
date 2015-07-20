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
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
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
namespace fs = boost::filesystem;
using boost::lexical_cast;

/** Access a single synapse file (nrn*.h5 or nrn*.h5.<int> */
class SynapseFile : public boost::noncopyable
{
public:
    explicit SynapseFile( const std::string& source )
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

    ~SynapseFile()
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

    SynapseMatrix read( const uint32_t gid, const uint32_t attributes ) const
    {
        switch( _numAttributes )
        {
        case SYNAPSE_ALL:
            return read< SYNAPSE_ALL >( gid, attributes );
        case SYNAPSE_POSITION_ALL:
            return read< SYNAPSE_POSITION_ALL >( gid, attributes );
        default:
            LBERROR << "Synapse file " << _file.getFileName()
                    << " has unknown number of attributes: " << _numAttributes
                    << std::endl;
            return SynapseMatrix();
        }
    }

private:
    H5::H5File _file;
    size_t _numAttributes;
};

/** Implement the logic to read a merged .h5 or individual .h5.<int> files */
class Synapse : public boost::noncopyable
{
public:
    explicit Synapse( const std::string& source )
        : _file( 0 )
        , _gid( 0 )
    {
        try
        {
            _file = new SynapseFile( source );
        }
        catch( const std::runtime_error& )
        {
            // try to open in individual files
            const fs::path dir = fs::path( source ).parent_path();
            const fs::path filename =  fs::path( source ).filename();
            const boost::regex filter( filename.string() + "\\.[0-9]+$" );

            fs::directory_iterator end;
            for( fs::directory_iterator i( dir ); i != end; ++i )
            {
                const fs::path candidate = i->path().filename();
                boost::smatch match;

                if( !boost::filesystem::is_regular_file( i->status( )) ||
                    !boost::regex_match( candidate.string(), match, filter ))
                {
                    continue;
                }

                _unmappedFiles.push_back( i->path().string( ));
            }

            if( _unmappedFiles.empty( ))
            {
                LBTHROW( std::runtime_error( "Could not find synapse files " +
                                             dir.string() + "/" +
                                             filename.string() + ".[0-9]+" ));
            }
        }
    }

    ~Synapse()
    {
        delete _file;
    }

    SynapseMatrix read( const uint32_t gid, const uint32_t attributes ) const
    {
        if( _findFile( gid ))
            return _file->read( gid, attributes );
        return SynapseMatrix();
    }

    size_t getNumSynapses( const GIDSet& gids ) const
    {
        size_t numSynapses = 0;
        BOOST_FOREACH( const uint32_t gid, gids )
        {
            if( !_findFile( gid ))
                continue;

            GIDSet set;
            set.insert( gid );
            numSynapses += _file->getNumSynapses( set );
        }
        return numSynapses;
    }

private:
    typedef std::map< uint32_t, std::string > GidFileMap;

    mutable SynapseFile* _file;
    mutable uint32_t _gid; // current or 0 for all
    mutable Strings _unmappedFiles;
    mutable GidFileMap _fileMap;

    bool _findFile( const uint32_t gid ) const
    {
        if( _file && ( _gid == gid || _gid == 0 ))
            return true;

        const std::string& filename = _findFilename( gid );
        if( filename.empty( ))
            return false;

        delete _file;
        _file = new SynapseFile( filename );
        _gid = gid;
        return true;
    }

    std::string _findFilename( const uint32_t gid ) const
    {
        while( _fileMap[ gid ].empty( ))
        {
            if( _unmappedFiles.empty( ))
                return std::string();

            const std::string candidate = _unmappedFiles.back();
            _unmappedFiles.pop_back();

            lunchbox::ScopedWrite mutex( detail::_hdf5Lock );
            H5::H5File file;
            try
            {
                SilenceHDF5 silence;
                file.openFile( candidate, H5F_ACC_RDONLY );
            }
            catch( const H5::Exception& exc )
            {
                LBINFO <<  "Could not open synapse file " << candidate << ": "
                       << exc.getDetailMsg() << std::endl;
                continue;
            }

            const size_t size = file.getNumObjs();
            for( size_t i = 0; i < size; ++i )
            {
                const std::string& name = file.getObjnameByIdx( i );
                const boost::regex filter( "^a[0-9]+$" );
                boost::smatch match;

                if( boost::regex_match( name, match, filter ))
                {
                    std::string string = match.str();
                    string.erase( 0, 1 ); // remove the 'a'
                    const uint32_t cGID = lexical_cast<uint32_t>( string );
                    _fileMap[ cGID ] = candidate;
                }
            }
            file.close();
        }

        return _fileMap[ gid ];
    }
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
    return _impl->read( gid, attributes );
}

size_t Synapse::getNumSynapses( const GIDSet& gids ) const
{
    return _impl->getNumSynapses( gids );
}

}
