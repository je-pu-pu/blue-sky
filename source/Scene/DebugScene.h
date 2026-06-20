#pragma once

#include "Scene.h"
#include <blue_sky/CityGenerator.h>
#include <core/graphics/TextStyle.h>
#include <memory>

namespace core::graphics
{
	class RenderTargetTexture;
}

namespace blue_sky
{

class Camera;

/**
 * デバッグ用シーン
 *
 */
class DebugScene : public Scene
{
public:
	static inline const char_t* name = "debug";

protected:
	CityGenerator city_generator_;

	std::unique_ptr< core::graphics::RenderTargetTexture > render_result_texture_;
	std::unique_ptr< core::graphics::RenderTargetTexture > velocity_texture_;	///< モーションベクトル ( 速度 ) G-buffer ( 案B / 段階0b )

	core::graphics::TextStyle debug_text_style_ = { Color::White, Color::Black, 2.f, 32.f };

	// フレームダンプ ( neural NPR オフライン検証用 )
	bool	frame_dumping_			= false;	///< ダンプ中か
	int		frame_dump_total_		= 120;		///< ダンプするフレーム数
	int		frame_dump_remaining_	= 0;		///< 残りフレーム数
	int		frame_dump_index_		= 0;		///< 出力中のフレーム番号
	bool	frame_dump_gbuffer_		= false;	///< 深度+カメラ行列/モーションベクトルも併せてダンプするか ( 案B / 段階0b )

	// モーションベクトル用の前フレームカメラ行列
	Matrix	prev_view_;
	Matrix	prev_projection_;
	bool	prev_camera_valid_		= false;

protected:

	/// @todo Scenegraph に移動する
	std::unique_ptr< Camera > camera_;

public:
	explicit DebugScene();
	~DebugScene() override;

	void update() override;
	void render() override;

	bool is_clip_cursor_required() override { return true; }
};

} // namespace blue_sky