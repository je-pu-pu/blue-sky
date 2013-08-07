#ifndef COMMON_CHASE_VALUE_H
#define COMMON_CHASE_VALUE_H

#include <common/math.h>

namespace common
{

template< typename Type, typename SpeedType = Type >
class chase_value
{
private:
	Type value_;
	Type target_value_;
	SpeedType speed_;

public:
	chase_value( Type value, Type target_value, SpeedType speed )
		: value_( value )
		, target_value_( target_value )
		, speed_( speed )
	{

	}

	chase_value( Type value, SpeedType speed )
		: value_( value )
		, target_value_( value )
		, speed_( speed )
	{

	}

	void chase()
	{
		value_ = math::chase( value_, target_value_, speed_ );
	}

	void chase_full()
	{
		value_ = target_value_;
	}

	Type& value() { return value_; }
	Type value() const { return value_; }

	Type& target_value() { return target_value_; }
	Type target_value() const { return target_value_; }
	
	SpeedType& speed() { return speed_; }
	SpeedType speed() const { return speed_; }

	// Type operator Type () { return value_; }
	// Type operator Type () const { return value_; }

}; // class chase_value

} // namespace common

#endif // COMMON_SERIALIZE_H
