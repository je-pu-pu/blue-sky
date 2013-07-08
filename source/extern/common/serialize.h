// *********************************************************
// serialize.h
// copyright (c) JE all rights reserved
// 2003/1/5
// *********************************************************

#ifndef COMMON_SERIALIZE_H
#define COMMON_SERIALIZE_H

#include <boost/lexical_cast.hpp>

namespace common
{

template< typename Type >
inline std::string serialize( Type x ) { return boost::lexical_cast< std::string >( x ); }

template< typename Type >
inline std::wstring w_serialize( Type x )
{
	std::wstringstream ss;
	std::wstring s;

	ss << x;
	
	return ss.str();
}

template< typename Type >
inline Type deserialize( const std::string& str ) { return boost::lexical_cast< Type >( str ); }

template<> inline std::string serialize( bool x ) { return x ? "true" : "false"; }
template<> inline bool deserialize( const std::string& str ) { return str == "true" ? true : false; }

} // namespace common

#endif // COMMON_SERIALIZE_H
