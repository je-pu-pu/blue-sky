#pragma once

#include <random>

namespace common
{

extern std::mt19937 random_engine;

inline void random_set_seed( int seed )
{
	random_engine.seed( seed );
}

/**
 * min <= n <= max のランダムな値を取得する
 *
 */
template< typename T > inline T random( T min, T max )
{
	std::uniform_real_distribution< T > dist( min, max );

	return dist( random_engine );
}

/**
 * min <= n < max のランダムな値を取得する
 *
 */
template<> inline int random< int >( int min, int max )
{
	std::uniform_int_distribution<> dist( min, max );

	return dist( random_engine );
}

} // namespace common
