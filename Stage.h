#ifndef BLUE_SKY_STAGE_H
#define BLUE_SKY_STAGE_H

#include "GridData.h"

namespace blue_sky
{

/**
 * ステージ
 *
 */
class Stage : public GridData
{
public:
	Stage( int, int );
	virtual ~Stage();
};

} // namespace blue_sky

#endif // BLUE_SKY_STAGE_H