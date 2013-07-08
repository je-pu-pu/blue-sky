#ifndef GAME_AABB_H
#define GAME_AABB_H

namespace game
{

template< typename T >
class AABB
{
public:

private:
	T min_;
	T max_;

public:
	AABB( const T& min, const T& max )
		: min_( min )
		, max_( max )
	{

	}

	AABB operator + ( const T& v )
	{
		return AABB( min_ + v, max_ + v );
	}

	const T& min() const { return min_; }
	const T& max() const { return max_; }

	bool collision_detection( const AABB& aabb ) const
	{
		if ( max().x() < aabb.min().x() || min().x() > aabb.max().x() ) return false;
		if ( max().y() < aabb.min().y() || min().y() > aabb.max().y() ) return false;
		if ( max().z() < aabb.min().z() || min().z() > aabb.max().z() ) return false;

		return true;
	}

	bool collision_detection( const T& v ) const
	{
		if ( v.x() < min.x() || v.x() > max.x() ) return false;
		if ( v.y() < min.y() || v.y() > max.y() ) return false;
		if ( v.z() < min.z() || v.z() > max.z() ) return false;

		return true;
	}

}; // class AABB

} // game

#endif // GAME_AABB_H