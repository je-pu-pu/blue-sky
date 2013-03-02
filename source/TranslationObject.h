#ifndef BLUE_SKY_TRANSLATION_OBJECT_H
#define BLUE_SKY_TRANSLATION_OBJECT_H

#include "StaticObject.h"
#include "type.h"

namespace blue_sky
{

/**
 * �ړ����Ȃ��I�u�W�F�N�g
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

	/// �X�V
	void update();

}; // class TranslationObject

} // namespace blue_sky

#endif // BLUE_SKY_TRANSLATION_OBJECT_H
