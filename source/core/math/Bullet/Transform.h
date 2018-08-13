#pragma once

#include "Quaternion.h"
#include <LinearMath/btTransform.h>

namespace core::bullet
{

class Transform
{
private:
	btTransform transform_;

public:
	Transform() = default;
	explicit Transform( const btTransform& t )
		: transform_( t )
	{ }

	void set_identity() { transform_.setIdentity(); }

	direct_x_math::Vector& get_position() { return reinterpret_cast< direct_x_math::Vector& >( transform_.getOrigin() ); }
	const direct_x_math::Vector& get_position() const { return reinterpret_cast< const direct_x_math::Vector& >( transform_.getOrigin() ); }
	void set_position( const direct_x_math::Vector& v ) { transform_.setOrigin( reinterpret_cast< const btVector3& >( v ) ); }

	Quaternion get_rotation() const { return Quaternion( transform_.getRotation() ); }
	void set_rotation( const Quaternion& q ) { transform_.setRotation( q ); }

	Transform operator * ( const Transform& t ) const { return Transform( transform_ * t.transform_ ); }

	operator btTransform& () { return transform_; }
	operator const btTransform& () const { return transform_; }
};

} // namespace core::bullet
