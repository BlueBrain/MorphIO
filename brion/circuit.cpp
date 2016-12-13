/* Copyright (c) 2013-2016, EPFL/Blue Brain Project
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

#include "circuit.h"

#include <bitset>
#include <boost/filesystem.hpp>
#include <boost/tokenizer.hpp>
#include <lunchbox/log.h>
#include <fstream>
#include <unordered_map>

namespace brion
{

enum Section
{
    SECTION_NEURONS,
    SECTION_MICROBOX,
    SECTION_LAYERS,
    SECTION_SYNAPSES,
    SECTION_ETYPES,
    SECTION_MTYPES,
    SECTION_MCPOSITIONS,
    SECTION_CIRCUITSEEDS,
    SECTION_UNKNOWN
};

class Circuit::Impl
{
public:
    explicit Impl( const std::string& source )
    {
        namespace fs = boost::filesystem;
        fs::path path = source;
        const std::string ext = fs::extension( path );
        if( ext != ".mvd" && ext != ".mvd2" )
            LBTHROW( std::runtime_error( "Expecting mvd file format for "
                                         " circuit file " + source ));

        typedef std::unordered_map< std::string, Section > LookUp;
        LookUp sections;
        sections.insert( std::make_pair( "Neurons Loaded", SECTION_NEURONS ));
        sections.insert( std::make_pair( "MicroBox Data", SECTION_MICROBOX ));
        sections.insert( std::make_pair( "Layers Positions Data",
                                         SECTION_LAYERS ));
        sections.insert( std::make_pair( "Axon-Dendrite Synapses",
                                         SECTION_SYNAPSES ));
        sections.insert( std::make_pair( "ElectroTypes", SECTION_ETYPES ));
        sections.insert( std::make_pair( "MorphTypes", SECTION_MTYPES ));
        sections.insert( std::make_pair( "MiniColumnsPosition",
                                         SECTION_MCPOSITIONS ));
        sections.insert( std::make_pair( "CircuitSeeds",
                                         SECTION_CIRCUITSEEDS ));

        _file.open( source.c_str( ));
        if( !_file.is_open( ))
            LBTHROW( std::runtime_error( "Could not open MVD2 file " + source));

        _file >> std::ws;
        Section current = SECTION_UNKNOWN;
        while( !_file.eof( ))
        {
            std::string content;
            std::getline( _file, content );
            _file >> std::ws;

            LookUp::const_iterator it = sections.find( content );
            if( it != sections.end() )
                current = it->second;
            else
                _table[current].push_back( content );
        }
    }

    NeuronMatrix get( const GIDSet& gids, const uint32_t attributes ) const
    {
        const std::bitset< NEURON_ALL > bits( attributes );
        if( !bits.any( ))
            return NeuronMatrix();

        const Strings& neurons = _table.find( SECTION_NEURONS )->second;

        std::vector<int32_t> indices;
        indices.reserve( gids.size( ));
        for( const uint32_t gid : gids )
        {
            if( gid > neurons.size( ) || gid == 0 )
            {
                std::stringstream msg;
                msg << "Cell GID out of range: " << gid;
                LBTHROW( std::runtime_error( msg.str().c_str( )));
            }
            indices.push_back( gid - 1 );
        }

        const size_t numNeurons =
            indices.empty() ? getNumNeurons() : indices.size();
        NeuronMatrix values( boost::extents[numNeurons][bits.count()] );
        std::vector<char> buffer;

        // This loop uses a hand-written string tokenizer to improve the
        // performance of loading very large circuits (millions of neurons).
        // This code is faster than using boost tokenizer because it does not
        // use std::string and does only very few memory allocations.
        for( size_t i = 0; i < numNeurons; ++i )
        {
            const size_t neuronIdx = indices.empty() ? i : indices[i];

            const std::string& line = neurons[neuronIdx];
            buffer.resize( std::max( buffer.size(), line.size() + 1 ));
            buffer[0] = 0;
            size_t bit = 0;
            size_t field = 0;
            size_t k = 0;
            for( unsigned int j = 0; j != line.size() + 1; ++j)
            {
                char c = line[j];
                if( c == ' ' || c == 0)
                {
                    // A new input field is stored in buffer (except for the
                    // null terminating character.
                    if( bits.test( bit ))
                    {
                        buffer[k] = 0;
                        values[i][field++] = buffer.data();
                    }
                    ++bit;
                    k = 0;
                    buffer[0] = 0;

                    // Skipping white space and stopping j at the position
                    // of the next character.
                    while( c == ' ' )
                        c = line[++j];
                }
                buffer[k++] = c;
            }
        }
        return values;
    }

    size_t getNumNeurons() const
    {
        return _table.find( SECTION_NEURONS )->second.size();
    }

    Strings getTypes( const NeuronClass type ) const
    {
        switch( type )
        {
        case NEURONCLASS_ETYPE:
        {
            CircuitTable::const_iterator i = _table.find( SECTION_ETYPES );
            return i != _table.end() ? i->second : Strings();
        }

        case NEURONCLASS_MTYPE:
        case NEURONCLASS_MORPHOLOGY_CLASS:
        case NEURONCLASS_FUNCTION_CLASS:
        {
            CircuitTable::const_iterator i = _table.find( SECTION_MTYPES );
            if( i == _table.end( ))
                return Strings();
            const Strings& data = i->second;
            Strings buffer( data.size( ));
            typedef boost::tokenizer< boost::char_separator< char > > tokenizer;
            boost::char_separator< char > sep( " " );
            for( size_t j = 0; j < buffer.size(); ++j )
            {
                const tokenizer tokens( data[j], sep );
                tokenizer::const_iterator t = tokens.begin();
                std::advance( t, int32_t(type));
                buffer[j] = *t;
            }
            return buffer;
        }

        case NEURONCLASS_INVALID:
        default:
            return Strings();
        }
    }

private:
    std::ifstream _file;

    typedef std::unordered_map< uint32_t, Strings > CircuitTable;
    CircuitTable _table;
};

Circuit::Circuit( const std::string& source )
    : _impl( new Impl( source ))
{
}

Circuit::Circuit( const URI& source )
    : _impl( new Impl( source.getPath( )))
{
}

Circuit::~Circuit()
{
    delete _impl;
}

NeuronMatrix Circuit::get( const GIDSet& gids, const uint32_t attributes ) const
{
    return _impl->get( gids, attributes );
}

size_t Circuit::getNumNeurons() const
{
    return _impl->getNumNeurons();
}

Strings Circuit::getTypes( const NeuronClass type ) const
{
    return _impl->getTypes( type );
}

}
