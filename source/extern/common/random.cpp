#include "random.h"

namespace common
{

std::mt19937 random_engine( [] () { std::random_device seed_gen; return seed_gen(); }() );

} // namespace common
