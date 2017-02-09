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

#include "target.h"

#include <lunchbox/log.h>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <fstream>
#include <unordered_map>

namespace boost
{
template<>
inline brion::TargetType lexical_cast( const std::string& s )
{
    if( s == "Cell" )
        return brion::TARGET_CELL;
    if( s == "Compartment" )
        return brion::TARGET_COMPARTMENT;
    throw boost::bad_lexical_cast();
}
}

using boost::lexical_cast;

namespace brion
{
namespace detail
{
class Target
{
public:
    explicit Target( const std::string& source )
    {
        std::ifstream file( source.c_str( ));
        if( !file.is_open( ))
            LBTHROW( std::runtime_error( "Cannot open target file " + source ));
        std::stringstream buffer;
        buffer << file.rdbuf();

        boost::regex commentregx( "#.*?\\n" );
        const std::string fileString = boost::regex_replace( buffer.str(),
                                                             commentregx , "" );

        boost::regex regx( "Target (?<type>[a-zA-Z0-9_]+) (?<name>.*?)\\s+"
                           "\\{(?<contents>.*?)\\}" );
        const int subs[] = {1, 2, 3};
        boost::sregex_token_iterator i( fileString.begin(), fileString.end(),
                                        regx, subs );
        for( boost::sregex_token_iterator j; i != j; )
        {
            const std::string& typeStr = *i++;
            const std::string& name = *i++;
            std::string content = *i++;

            const TargetType type = lexical_cast< TargetType >( typeStr );
            _targetNames[type].push_back( name );
            boost::trim( content );
            if( content.empty( ))
                _targetValues[name] = Strings();
            else
            {
                boost::split( _targetValues[name], content,
                              boost::is_any_of("\n "),
                              boost::token_compress_on );
            }
        }

        if( _targetNames.empty( ))
            LBTHROW( std::runtime_error( source + " not a valid target file" ));
    }

    const Strings& getTargetNames( const TargetType type ) const
    {
        NameTable::const_iterator i = _targetNames.find( type );
        if( i != _targetNames.end( ))
            return i->second;
        static Strings empty;
        return empty;
    }

    bool contains( const std::string& name ) const
        { return _targetValues.find( name ) != _targetValues.end(); }

    const Strings& get( const std::string& name ) const
    {
        ValueTable::const_iterator i = _targetValues.find( name );
        if( i != _targetValues.end( ))
            return i->second;
        throw( std::runtime_error( name + " not a valid target" ));
    }

private:
    typedef std::unordered_map< uint32_t, Strings > NameTable;
    typedef std::unordered_map< std::string, Strings > ValueTable;
    NameTable _targetNames;
    ValueTable _targetValues;
};
}

Target::Target( const Target& from )
    : _impl( new detail::Target( *from._impl ))
{}

Target::Target( const std::string& source )
    : _impl( new detail::Target( source ))
{
}

Target::~Target()
{
    delete _impl;
}

Target& Target::operator = ( const Target& rhs )
{
    if( this == &rhs )
        return *this;

    delete _impl;
    _impl = new detail::Target( *rhs._impl );
    return *this;
}


const Strings& Target::getTargetNames( const TargetType type ) const
{
    return _impl->getTargetNames( type );
}

bool Target::contains( const std::string& name ) const
{
    return _impl->contains( name );
}

const Strings& Target::get( const std::string& name ) const
{
    return _impl->get( name );
}

GIDSet Target::parse( const Targets& targets, const std::string& root )
{
    if( root.empty( ))
        LBTHROW( std::runtime_error( "Empty target name" ));

    GIDSet gids;
    Strings names( 1, root );
    while( !names.empty( ))
    {
        const std::string name = names.back();
        names.pop_back();

        if( name[0] == 'a' ) // maybe a GID
        {
            try
            {
                gids.insert( lexical_cast< uint32_t >( name.substr( 1 )));
                continue;
            }
            catch( const boost::bad_lexical_cast& ) {} // not a GID
        }

        bool found = false;
        BOOST_FOREACH( const Target& target, targets )
        {
            if( !target.contains( name ))
                continue;

            const brion::Strings& values = target.get( name );
            std::copy( values.begin(), values.end(),
                       std::back_inserter( names ));
            found = true;
            break;
        }
        if( !found )
            LBTHROW( std::runtime_error( "Parse " + root + " failed: " + name +
                                         " is not a valid or known target" ));
    }

    return gids;
}

std::ostream& operator << ( std::ostream& os, const Target& target )
{
    const Strings& targetNames = target.getTargetNames( brion::TARGET_CELL );
    BOOST_FOREACH( const std::string& name, targetNames )
    {
        const Strings& values = target.get( name );
        os << "Target " << name << ": ";
        BOOST_FOREACH( const std::string& value, values )
        {
            os << value << " ";
        }
        os << std::endl;
    }
    return os << std::endl;
}

}
