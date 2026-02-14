#pragma once

#include "StaticObject.h"
#include <type/type.h>

namespace blue_sky
{

/**
 * 周期的に移動するオブジェクト（動く床など）
 *
 * StaticObject を基底に sin/cos で位置を振動させる。
 * 物理的には kinematic body として扱われる。
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
