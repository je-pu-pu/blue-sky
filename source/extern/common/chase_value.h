#ifndef COMMON_CHASE_VALUE_H
#define COMMON_CHASE_VALUE_H

#include <common/math.h>

namespace common
{

template< typename Type >
class chase_value
{
private:
	Type value_;
	Type target_value_;
	Type speed_;

public:
	chase_value( Type value, Type target_value, Type speed )
		: value_( value )
		, target_value_( target_value )
		, speed_( speed )
	{

	}

	void chase()
	{
		value_ = math::chase( value_, target_value_, speed_ );
	}

	Type& value() { return value_; }
	Type value() const { return value_; }

	Type& target_value() { return target_value_; }
	Type target_value() const { return target_value_; }
	
	Type& speed() { return speed_; }
	Type speed() const { return speed_; }

	// Type operator Type () { return value_; }
	// Type operator Type () const { return value_; }

}; // class chase_value

} // namespace common

#endif // COMMON_SERIALIZE_H
