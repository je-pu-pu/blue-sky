#pragma once

#include "Scene.h"
#include <blue_sky/CityGenerator.h>
#include <core/graphics/TextStyle.h>
#include <memory>
#include <vector>
#include <random>
#include <cstdint>

namespace core::graphics
{
	class RenderTargetTexture;
	class NeuralStylizer;

	namespace direct_3d_11 { class Texture; }
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

	// Neural NPR リアルタイム推論 ( ONNX Runtime + DirectML )
	std::unique_ptr< core::graphics::NeuralStylizer > neural_stylizer_;
	bool	neural_npr_enabled_		= false;	///< リアルタイム NPR を有効にするか
	std::unique_ptr< core::graphics::direct_3d_11::Texture > neural_output_texture_;	///< 推論結果の表示用(CPUからUpdateSubresourceで動的更新)
	// Neural NPR 用 CPU バッファ＆ノイズ状態
	std::vector< float >   neural_input_;            ///< 入力 NCHW float[0,1] (3*w*h)
	std::vector< float >   neural_noise_;            ///< 推論に渡すノイズ (w*h)
	std::vector< float >   neural_noise_prev_;       ///< 時間平滑(EMA)用の前フレーム状態 (w*h)
	std::vector< float >   neural_output_;           ///< 出力 NCHW float (3*w*h)
	std::vector< uint8_t > neural_rgba_;             ///< 出力 RGBA8 (4*w*h)
	std::mt19937           neural_rng_{ 12345u };    ///< ノイズ生成器
	float                  neural_noise_temporal_ = 0.35f;  ///< 時間平滑係数(EMA, 小さいほど滑らか=ボイリング弱)
	int                    neural_proc_long_side_ = 384;    ///< 推論する処理解像度の長辺(px)。小さいほど高速・粗い。表示時にGPUで画面サイズへ拡大

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