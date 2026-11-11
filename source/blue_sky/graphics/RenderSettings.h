#pragma once

#include <core/type.h>

namespace blue_sky::graphics
{

/**
 * 描画の設定の窓口（空・地面・環境色・影の色・紙・線の種類・線の揺れ）
 *
 * ステージの動き（stage::StageLook）・ステージを当てる所（stage::StageBuilder）・シーンの線の揺れが使う。
 * GraphicsManager が実装する。使う側はこれだけを受け取るので、テストでは偽物を渡して呼ばれ方を確かめられる
 * （以前は GraphicsManager の全体を受け取り、偽物を作るには純粋仮想を約 70 個埋める必要があった。設計上の問題 D3）
 */
class RenderSettings
{
public:
	virtual ~RenderSettings() = default;

	/// 空（スカイボックス）を名前で設定する・外す
	virtual void set_sky_box( const char_t* name ) = 0;
	virtual void unset_sky_box() = 0;

	/// 地面を名前で設定する
	virtual void set_ground( const char_t* name ) = 0;

	virtual void set_ambient_color( const core::Color& ) = 0;
	virtual void set_shadow_color( const core::Color& ) = 0;
	virtual void set_shadow_paper_color( const core::Color& ) = 0;

	/// 線の揺れの大きさ（音楽の大きさに合わせて毎フレーム変える）
	virtual void set_drawing_accent( float_t ) = 0;

	/// 線の種類・紙の種類（番号）
	virtual void set_drawing_line_type( int_t ) = 0;
	virtual void set_paper_texture_type( int_t ) = 0;
};

} // namespace blue_sky::graphics
