#pragma once

#include <blue_sky/graphics/Model.h>

namespace blue_sky::graphics
{

/**
 * 画面をフェードイン, フェードアウトさせる
 *
 */
class Fader : public Model
{
private:
	Color			fade_color_;		///< フェードアウト時の色
	Color			current_color_;		///< 現在の色
	float_t			fade_;				///< フェード値 ( 0.f .. 1.f )

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
