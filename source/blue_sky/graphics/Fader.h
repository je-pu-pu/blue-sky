#pragma once

#include <blue_sky/graphics/Model.h>

namespace blue_sky::graphics
{

/**
 * ��ʂ��t�F�[�h�C��, �t�F�[�h�A�E�g������
 *
 */
class Fader : public Model
{
private:
	Color			fade_color_;		///< �t�F�[�h�A�E�g���̐F
	Color			current_color_;		///< ���݂̐F
	float_t			fade_;				///< �t�F�[�h�l ( 0.f .. 1.f )

	void update_color();
public:
	Fader();

	bool fade_in( float_t = 0.05f );
	bool fade_out( float_t = 0.05f );

	void full_in();
	void full_out();

	bool is_full_in() const { return fade_ == 0.f; }
	bool is_full_out() const { return fade_ == 1.f; }

	void set_color( const Color& c ) { fade_color_ = c; current_color_ = Color( c.r(), c.g(), c.b(), current_color_.a() ); }
	const Color& get_color() const { return current_color_; }

}; // class Fader

} // namespace blue_sky::graphics
