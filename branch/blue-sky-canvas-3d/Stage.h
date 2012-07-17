#ifndef BLUE_SKY_STAGE_H
#define BLUE_SKY_STAGE_H

#include "GridData.h"
#include "vector3.h"
#include <game/AABB.h>
#include <vector>

namespace blue_sky
{

/**
 * ステージ
 *
 */
class Stage : public GridData
{
public:
	typedef game::AABB< vector3 > AABB;
	typedef std::vector< AABB* > AABBList;

private:
	AABBList aabb_list_;

public:
	Stage( int, int );
	virtual ~Stage();

	const AABB* get_collision_aabb( const AABB& ) const;
};

} // namespace blue_sky

#endif // BLUE_SKY_STAGE_H