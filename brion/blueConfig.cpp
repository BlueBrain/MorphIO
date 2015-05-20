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

#include "blueConfig.h"

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <lunchbox/log.h>
#include <lunchbox/stdExt.h>

namespace boost
{
template<>
inline brion::BlueConfigSection lexical_cast( const std::string& s )
{
    if( s == "Run" )
        return brion::CONFIGSECTION_RUN;
    if( s == "Connection" )
        return brion::CONFIGSECTION_CONNECTION;
    if( s == "Stimulus" )
        return brion::CONFIGSECTION_STIMULUS;
    if( s == "StimulusInject" )
        return brion::CONFIGSECTION_STIMULUSINJECT;
    if( s == "Report" )
        return brion::CONFIGSECTION_REPORT;
    return brion::CONFIGSECTION_UNKNOWN;
}

template<>
inline std::string lexical_cast( const brion::BlueConfigSection& b )
{
    switch( b )
    {
    case brion::CONFIGSECTION_RUN:
        return "Run";
    case brion::CONFIGSECTION_CONNECTION:
        return "Connection";
    case brion::CONFIGSECTION_STIMULUS:
        return "Stimulus";
    case brion::CONFIGSECTION_STIMULUSINJECT:
        return "StimulusInject";
    case brion::CONFIGSECTION_REPORT:
        return "Report";
    default:
        return "UNKNOWN";
    }
    throw boost::bad_lexical_cast();
}
}

namespace brion
{

typedef stde::hash_map< std::string, std::string > KVStore;
typedef stde::hash_map< std::string, KVStore > ValueTable;

namespace detail
{
class BlueConfig
{
public:
    explicit BlueConfig( const std::string& source )
    {
        std::ifstream file( source.c_str( ));
        if( !file.is_open( ))
            LBTHROW( std::runtime_error( "Cannot open BlueConfig file " +
                                         source ));
        std::stringstream buffer;
        buffer << file.rdbuf();

        boost::regex commentregx( "#.*?\\n" );
        const std::string fileString = boost::regex_replace( buffer.str(),
                                                           commentregx , "\n" );

        boost::regex regx( "(?<type>[a-zA-Z0-9_-]+) (?<name>[a-zA-Z0-9_-]+?)"
                           "\\s+\\{(?<contents>.*?)\\}" );
        const int subs[] = {1, 2, 3};
        boost::sregex_token_iterator i( fileString.begin(), fileString.end(),
                                        regx, subs );
        for( boost::sregex_token_iterator j; i != j; )
        {
            const std::string& typeStr = *i++;
            const std::string& name = *i++;
            const std::string& content = *i++;
            if( content.empty( ))
            {
                LBWARN << "Found empty section '" << typeStr << " " << name
                       << "' in BlueConfig file " << source << std::endl;
                continue;
            }

            const BlueConfigSection type =
                boost::lexical_cast< BlueConfigSection >( typeStr );
            if( type == brion::CONFIGSECTION_UNKNOWN )
            {
                LBWARN << "Found unknown section '" << typeStr
                       << "' in BlueConfig file " << source << std::endl;
                continue;
            }

            _names[type].push_back( name );

            Strings lines;
            boost::split( lines, content, boost::is_any_of( "\n" ),
                          boost::token_compress_on );

            BOOST_FOREACH( std::string line, lines )
            {
                boost::trim( line );
                if( line.empty( ))
                    continue;

                const std::string::size_type pos = line.find( ' ', 0 );
                if( pos == std::string::npos )
                {
                    LBWARN << "Found invalid key-value pair '" << line
                           << "' in BlueConfig file " << source << std::endl;
                    continue;
                }

                std::string value = line.substr( pos+1 );
                boost::trim( value );
                _table[type][name].insert( std::make_pair( line.substr( 0, pos),
                                                           value ));
            }
        }

        if( _table[CONFIGSECTION_RUN].empty( ))
            LBTHROW( std::runtime_error( source +
                                         " not a valid BlueConfig file" ));
    }

    Strings _names[CONFIGSECTION_ALL];
    ValueTable _table[CONFIGSECTION_ALL];
};
}

BlueConfig::BlueConfig( const std::string& source )
    : _impl( new detail::BlueConfig( source ))
{
}

BlueConfig::~BlueConfig()
{
    delete _impl;
}

const Strings&
BlueConfig::getSectionNames( const BlueConfigSection section ) const
{
    return _impl->_names[section];
}

const std::string& BlueConfig::get( const BlueConfigSection section,
                                    const std::string& sectionName,
                                    const std::string& key ) const
{
    return _impl->_table[section][sectionName][key];
}

std::ostream& operator << ( std::ostream& os, const BlueConfig& config )
{
    for( size_t i = 0; i < CONFIGSECTION_ALL; ++i )
    {
        BOOST_FOREACH( const ValueTable::value_type& entry,
                       config._impl->_table[i] )
        {
            os << boost::lexical_cast< std::string >( BlueConfigSection( i ))
               << " " << entry.first << std::endl;
            BOOST_FOREACH( const KVStore::value_type& pair, entry.second )
            {
                os << "   " << pair.first << " " << pair.second << std::endl;
            }
            os << std::endl;
        }
    }

    return os;
}

}
