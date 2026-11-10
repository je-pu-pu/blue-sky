#pragma once

#include <core/type.h>
#include <common/chase_value.h>

namespace blue_sky::graphics
{
	class RenderSettings;
}

namespace blue_sky::stage
{

struct Action;

/**
 * ステージの見た目（光の位置・環境色・影の色・紙の影の色・線の揺れの大きさ）と、それを変えるステージの動き（set・fade）
 *
 * 以前は GamePlayScene のメンバ（ambient_color_ など）と run_action の中に混ざっていて、シーンごと作らないと試せなかった。
 * 描画の設定は小さな窓口（graphics::RenderSettings）で受け取るので、偽物で呼ばれ方を確かめられる（設計上の問題 D3）。
 *
 * 色と光の位置は「追う値」で持つ。set はすぐ目標の値にし、fade は目標の値だけ変えて毎フレーム chase で近づける。
 */
class StageLook
{
public:
	using Color = core::Color;
	using Vector3 = core::Vector3;

private:
	common::chase_value< Vector3, float_t >		light_position_;
	common::chase_value< Color, float_t >		ambient_color_;
	common::chase_value< Color, float_t >		shadow_color_;
	common::chase_value< Color, float_t >		shadow_paper_color_;

	float_t		drawing_accent_scale_ = 0.f;	///< ( 0 : アクセントを完全に無効化 ) .. ( 2 : アクセント 2 倍 ) ..
	bool		shading_enabled_ = true;		///< ステージの shading_enabled（読むだけで、今は描画に使っていない。以前の GamePlayScene と同じ）

public:
	StageLook();

	/**
	 * ステージの動きのうち見た目に関わるものを行う（set・fade。ほかの種類と、set の音・BPM などは何もしない）
	 *
	 * set : 空（"none" なら外す）・線の種類・紙の種類はすぐ描画の設定に当てる。色・光の位置はすぐその値にする（色は書いた成分だけ）。
	 * fade : 色・光の位置の目標だけを変える（speed を書いたときは色の追う速さも変える）
	 */
	void apply( const Action& action, graphics::RenderSettings& settings );

	/// 色・光の位置を目標へ 1 フレーム分近づける（シーンの update で毎フレーム呼ぶ）
	void chase();

	/// 環境色を描画の設定に当てる（シーンの update で chase の後に呼ぶ。以前の GamePlayScene::update と同じ所）
	void apply_ambient_color( graphics::RenderSettings& settings ) const;

	/// 影の色・紙の影の色・線の揺れ（音の大きさ × 揺れの大きさ）を描画の設定に当てる（シーンの render で呼ぶ）
	void apply_shadow_and_accent( graphics::RenderSettings& settings, float_t peak_level ) const;

	Vector3 get_light_position() const { return light_position_.value(); }
	Color get_ambient_color() const { return ambient_color_.value(); }
	Color get_shadow_color() const { return shadow_color_.value(); }
	Color get_shadow_paper_color() const { return shadow_paper_color_.value(); }
	float_t get_drawing_accent_scale() const { return drawing_accent_scale_; }
	bool is_shading_enabled() const { return shading_enabled_; }
};

} // namespace blue_sky::stage
