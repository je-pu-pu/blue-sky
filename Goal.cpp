#include "Goal.h"

namespace blue_sky
{

Goal::Goal()
{
	local_aabb_list().clear();
	local_aabb_list().push_back( AABB( vector3( -1, -1, -1 ), vector3( 1, 1, 1 ) ) );
}

/**
 * �X�V
 *
 */
void Goal::update()
{

}

} // namespace blue_sky
