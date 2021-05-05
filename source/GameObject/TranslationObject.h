#pragma once

#include "StaticObject.h"
#include <type/type.h>

namespace blue_sky
{

/**
 * 移動しないオブジェクト @todo 名前を直す StaticObject から継承してるのは変
 *
 */
class TranslationObject : public StaticObject
{
private:
	float_t tw_, th_, td_;
	float speed_;
	float a_;

protected:
	

public:
	TranslationObject( float_t, float_t, float_t, float_t, float_t, float_t, float_t );
	~TranslationObject();

	/// 更新
	void update();

}; // class TranslationObject

} // namespace blue_sky
