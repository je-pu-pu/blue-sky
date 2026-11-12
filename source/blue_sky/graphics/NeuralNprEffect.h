#pragma once

#include <core/ResourceHandle.h>

#include <memory>
#include <vector>
#include <random>
#include <cstdint>

namespace core::graphics
{
	class GraphicsManager;
	class RenderTargetTexture;
	class NeuralStylizer;
	class Shader;
	class Texture;
}

namespace blue_sky::graphics
{

/**
 * Neural NPR リアルタイム推論（ONNX Runtime + DirectML）＋色保持リカラーのポストエフェクト。
 *
 * シーン描画結果テクスチャを CPU 読み戻し → 処理解像度へダウンサンプル → フレーム毎ノイズ
 * 生成 → スタイル化推論 → 色保持リカラー（or 素通し）で表示、までを担う。
 *
 * DebugScene から NPR パイプラインを分離したもの。状態（モデル・バッファ・ノイズ・
 * パラメータ・出力テクスチャ）を一手に保持し、他シーンからも再利用できる。
 */
class NeuralNprEffect
{
public:
	/// モデルを DirectML でロードし、色保持/素通しシェーダを取得する（gm は setup_default_shaders 済みであること）。
	explicit NeuralNprEffect( core::graphics::GraphicsManager* gm );

	/// unique_ptr メンバ（不完全型）のため、デストラクタは .cpp（完全型がある側）で定義する。
	~NeuralNprEffect();

	/// モデルのロードに成功しているか。
	bool is_loaded() const;

	/**
	 * scene をスタイル化してバックバッファへ表示する。
	 * @return 表示したら true（呼び出し側は通常のポストエフェクト表示を省略する）。
	 *         無効時・モデル未ロード時・失敗時は false（何も描かない）。
	 */
	bool apply( core::graphics::GraphicsManager* gm, core::graphics::RenderTargetTexture* scene );

	/// imgui のデバッグ UI（トグル・スライダ・計測表示）を描画する。
	void render_imgui();

private:
	void generate_noise( int width, int height );  ///< フレーム毎ノイズ（空間白色＋時間 EMA 平滑）を noise_ に生成

	std::unique_ptr< core::graphics::NeuralStylizer >        stylizer_;
	std::unique_ptr< core::graphics::Texture > output_texture_;  ///< 推論結果の表示用（CPU から動的更新。GraphicsManager::create_texture で作る）
	/// 使うシェーダー（ハンドルで持ち、使うたびに引く。以前は生ポインタで、シェーダーが解放されると壊れ得た）
	core::ResourceHandle< core::graphics::Shader > recolor_shader_;  ///< 色保持リカラー用
	core::ResourceHandle< core::graphics::Shader > default_shader_;  ///< 素通し（うねり回避）

	// パラメータ
	bool  enabled_        = false;  ///< リアルタイム NPR を有効にするか
	bool  use_fp16_       = true;   ///< fp16 推論か(off=fp32)。約1.6倍速・画質ほぼ同等
	bool  loaded_fp16_    = true;   ///< ロード済みモデルが fp16 か（再ロード判定用）
	int   proc_long_side_ = 768;    ///< 推論する処理解像度の長辺(px)。768=画質と速度のバランス点
	float noise_temporal_ = 0.35f;  ///< 時間平滑係数(EMA, 小さいほど滑らか=ボイリング弱)
	bool  color_preserve_ = true;   ///< 色保持を使うか(off=ニューラル出力そのまま)
	float color_blue_     = 3.0f;   ///< 青方向の増幅
	float color_yellow_   = 0.0f;   ///< 黄方向の倍率
	float color_a_        = 0.5f;   ///< a 軸スプレッド
	float color_dark_     = 60.0f;  ///< 暗化許容(L)

	// CPU バッファ＆ノイズ状態
	std::vector< float >         input_;        ///< 入力 NCHW float[0,1] (3*w*h)
	std::vector< float >         noise_;        ///< 推論に渡すノイズ (w*h)
	std::vector< float >         noise_prev_;   ///< 時間平滑(EMA)用の前フレーム状態 (w*h)
	std::vector< float >         output_;       ///< 出力 NCHW float (3*w*h)
	std::vector< std::uint8_t >  rgba_;         ///< 出力 RGBA8 (4*w*h)
	std::vector< float >         gauss_table_;  ///< 事前計算 N(0,1) ノイズテーブル(高速化用)
	std::mt19937                 rng_{ 12345u };

	// 計測(ms)
	float ms_readback_ = 0.f, ms_pre_ = 0.f, ms_infer_ = 0.f, ms_post_ = 0.f, ms_total_ = 0.f;

}; // class NeuralNprEffect

} // namespace blue_sky::graphics
