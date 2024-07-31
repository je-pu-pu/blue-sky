#ifndef COMMON_TIMER_H
#define COMMON_TIMER_H

#include <boost/timer/timer.hpp>

namespace common
{

using timer = boost::timer::cpu_timer;

} // namespace common

#endif // COMMON_TIMER_H