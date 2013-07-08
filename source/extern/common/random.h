// *********************************************************
// Random.h
// copyright (c) JE all rights reserved
// 2002/12/27
// *********************************************************

#ifndef COMMON_RANDOM_H
#define COMMON_RANDOM_H

#include <boost/random.hpp>

namespace common
{

// typedef boost::minstd_rand GenerateType;
// typedef boost::uniform_real< GenerateType > Random;

template< typename T > inline T random( T min, T max ) { return rand() / static_cast< T >( RAND_MAX ) * ( max - min ) + min; }
template<> inline int random< int >( int min, int max ) { return static_cast< int >( rand() / static_cast< float >( RAND_MAX + 1 ) * ( max - min + 1 ) ) + min; }

} // namespace common

#endif // COMMON_RANDOM_H
