#pragma once

#include <blue_sky/type.h>

namespace blue_sky
{

/**
 * 画面をフェードイン, フェードアウトさせる（ゲームの演出の状態）
 *
 * 色・不透明度・フェードの進みだけを持つ。描くのは描画の窓口（GraphicsManager::render_screen_color に get_color() を渡す。シーンは Scene::render_fader）。
 * 持ち主は GameMain（シーンは Scene::get_fader で得る）。
 * 以前は描画の窓口（GraphicsManager）が持つ Model で、ゲームの演出の状態を描画の側が持っていた
 */
class Fader
{
private:
	Color			fade_color_;		///< フェードアウト時の色
	Color			current_color_;		///< 現在の色
	float_t			fade_;				///< フェード値 ( 0.f .. 1.f )

	bool_t			is_fading_in_ = true;	///< 毎フレームの進め方（update）。true : フェードイン or false : フェードアウト
	float_t			fade_speed_ = 0.f;		///< 毎フレームの進め方（update）の速さ

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

	/// 毎フレームの進め方を決める（Lua の start_fade_in・start_fade_out。進めるのは update）
	void start_fade_in( float_t speed ) { is_fading_in_ = true; fade_speed_ = speed; }
	void start_fade_out( float_t speed ) { is_fading_in_ = false; fade_speed_ = speed; }

	/// start_fade_in・start_fade_out で決めた向きと速さで 1 フレーム進める（使うシーンが毎フレーム呼ぶ）
	void update();

}; // class Fader

} // namespace blue_sky
