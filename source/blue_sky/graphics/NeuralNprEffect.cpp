#include "NeuralNprEffect.h"

#include "CoherentNoise.h"

#include <blue_sky/DebugLog.h>

#include <core/graphics/GraphicsManager.h>
#include <core/graphics/NeuralStylizer.h>
#include <core/graphics/Shader.h>
#include <core/graphics/Texture.h>
#include <core/graphics/RenderTargetTexture.h>

#include <imgui.h>

#include <filesystem>
#include <system_error>
#include <string>
#include <chrono>
#include <cmath>

namespace blue_sky::graphics
{

namespace
{
	// モデル配置は media/neural/（gitignore）。手順は tools/x64-deps/README ③ / tools/npr_offline/to_fp16.py
	constexpr const char* MODEL_FP16 = "media/neural/starry_boil_small_fp16.onnx";
	constexpr const char* MODEL_FP32 = "media/neural/starry_boil_small.onnx";

	constexpr int    PROC_ALIGN       = 16;                          ///< モデルは H/W が 16 の倍数必須
	constexpr size_t GAUSS_TABLE_SIZE = static_cast< size_t >( 1 ) << 21;  ///< 事前計算ノイズ表 ≈8MB
} // namespace

NeuralNprEffect::NeuralNprEffect( core::graphics::GraphicsManager* gm )
{
	// 色保持リカラー／素通しシェーダを取得（setup_default_shaders 済み前提）
	recolor_shader_ = gm->get_shader_handle( "post_effect_recolor" );
	default_shader_ = gm->get_shader_handle( "post_effect_default" );

	// モデルがあれば DirectML でロード（無ければ NPR 無効のまま続行）
	std::error_code ec;
	if ( std::filesystem::exists( MODEL_FP16, ec ) )
	{
		try
		{
			stylizer_ = std::make_unique< core::graphics::NeuralStylizer >( MODEL_FP16 );
			debug_log( "[NeuralNPR] モデルを読み込みました" );
		}
		catch ( const std::exception& e )
		{
			debug_log( std::string( "[NeuralNPR] モデルを読み込めませんでした: " ) + e.what() );
		}
	}
	else
	{
		debug_log( "[NeuralNPR] モデルのファイルがありません（作業ディレクトリを確認）: media/neural/starry_boil_small_fp16.onnx" );
	}
}

NeuralNprEffect::~NeuralNprEffect() = default;  // unique_ptr の不完全型はここ（完全型）で解決

bool NeuralNprEffect::is_loaded() const
{
	return stylizer_ != nullptr;
}

void NeuralNprEffect::generate_noise( int width, int height )
{
	// 空間白色 + 時間 EMA 平滑 -> std≈1 に正規化（infer_stochastic.py 相当）。
	// 高速化: 事前計算した N(0,1) テーブルを毎フレーム別オフセットで読み、EMA + 固定スケール。
	const size_t n = static_cast< size_t >( width ) * height;
	noise_.resize( n );
	if ( noise_prev_.size() != n ) noise_prev_.assign( n, 0.f );

	if ( gauss_table_.size() != GAUSS_TABLE_SIZE )
	{
		gauss_table_.resize( GAUSS_TABLE_SIZE );
		std::normal_distribution< float > g( 0.f, 1.f );
		for ( size_t i = 0; i < GAUSS_TABLE_SIZE; ++i ) gauss_table_[ i ] = g( rng_ );
	}

	// EMA 平滑＋std≈1 正規化は自由関数へ（テスト可能 / CoherentNoiseTest 参照）
	const size_t off = static_cast< size_t >( rng_() ) & ( GAUSS_TABLE_SIZE - 1 );
	step_coherent_noise( gauss_table_, noise_prev_, noise_, off, noise_temporal_ );
}

bool NeuralNprEffect::apply( core::graphics::GraphicsManager* gm, core::graphics::RenderTargetTexture* scene )
{
	using clock = std::chrono::high_resolution_clock;
	const auto msf = []( auto s, auto e ) { return static_cast< float >( std::chrono::duration< double, std::milli >( e - s ).count() ); };

	// fp16/fp32 切り替え: トグル変化時のみモデル再ロード（セッション再生成は重い）
	if ( use_fp16_ != loaded_fp16_ )
	{
		const char* m = use_fp16_ ? MODEL_FP16 : MODEL_FP32;
		try
		{
			stylizer_ = std::make_unique< core::graphics::NeuralStylizer >( m );
		}
		catch ( const std::exception& e )
		{
			debug_log( std::string( "[NeuralNPR] モデルを読み直せませんでした: " ) + e.what() );
		}
		loaded_fp16_ = use_fp16_;  // 失敗してもループしないようフラグは合わせる
	}

	if ( ! enabled_ || ! stylizer_ ) return false;

	// 描画結果を CPU へ読み戻す（以前は Direct3D 11 の描画先へキャストし、デバイスで直接読んでいた）
	std::vector< std::uint8_t > rgba;
	uint_t w = 0, h = 0;
	const auto t0 = clock::now();
	const bool ok = gm->read_texture_rgba8( scene, rgba, w, h );
	const auto t1 = clock::now();

	bool displayed = false;
	if ( ok )
	{
		// 処理解像度を下げる（長辺 = proc_long_side_）。推論/ノイズ/変換を一括削減。表示は GPU が拡大。
		const int full_w = static_cast< int >( w ), full_h = static_cast< int >( h );
		const int long_side = ( full_w > full_h ) ? full_w : full_h;
		const int target = ( proc_long_side_ > 0 ) ? proc_long_side_ : long_side;
		const float dscale = ( target < long_side ) ? static_cast< float >( target ) / long_side : 1.f;
		const int pw_raw = ( dscale < 1.f ) ? static_cast< int >( full_w * dscale ) : full_w;
		const int ph_raw = ( dscale < 1.f ) ? static_cast< int >( full_h * dscale ) : full_h;
		const int pw = ( ( pw_raw / PROC_ALIGN ) * PROC_ALIGN >= PROC_ALIGN ) ? ( pw_raw / PROC_ALIGN ) * PROC_ALIGN : PROC_ALIGN;
		const int ph = ( ( ph_raw / PROC_ALIGN ) * PROC_ALIGN >= PROC_ALIGN ) ? ( ph_raw / PROC_ALIGN ) * PROC_ALIGN : PROC_ALIGN;
		const size_t n = static_cast< size_t >( pw ) * ph;
		input_.resize( 3 * n );
		output_.resize( 3 * n );
		rgba_.resize( 4 * n );

		// フル解像度 RGBA8 -> 処理解像度 NCHW float[0,1]（最近傍ダウンサンプル）
		for ( int py = 0; py < ph; ++py )
		{
			const uint_t sy = static_cast< uint_t >( ( static_cast< size_t >( py ) * full_h ) / ph );
			for ( int px = 0; px < pw; ++px )
			{
				const uint_t sx = static_cast< uint_t >( ( static_cast< size_t >( px ) * full_w ) / pw );
				const size_t si = ( static_cast< size_t >( sy ) * full_w + sx ) * 4;
				const size_t di = static_cast< size_t >( py ) * pw + px;
				input_[ 0 * n + di ] = rgba[ si + 0 ] / 255.f;
				input_[ 1 * n + di ] = rgba[ si + 1 ] / 255.f;
				input_[ 2 * n + di ] = rgba[ si + 2 ] / 255.f;
			}
		}

		const auto t1b = clock::now();
		generate_noise( pw, ph );

		try
		{
			const auto t2 = clock::now();
			stylizer_->stylize( pw, ph, input_.data(), noise_.data(), output_.data() );
			const auto t3 = clock::now();

			// NCHW float -> RGBA8
			const auto to_u8 = []( float fv ) { const int x = static_cast< int >( fv * 255.f + 0.5f ); return static_cast< std::uint8_t >( x < 0 ? 0 : ( x > 255 ? 255 : x ) ); };
			for ( size_t i = 0; i < n; ++i )
			{
				rgba_[ 4 * i + 0 ] = to_u8( output_[ 0 * n + i ] );
				rgba_[ 4 * i + 1 ] = to_u8( output_[ 1 * n + i ] );
				rgba_[ 4 * i + 2 ] = to_u8( output_[ 2 * n + i ] );
				rgba_[ 4 * i + 3 ] = 255;
			}

			// 表示用テクスチャ（無し/サイズ違いなら作り直し）。処理解像度のまま、表示時に GPU が画面へ bilinear 拡大。
			if ( ! output_texture_ || static_cast< int >( output_texture_->get_width() ) != pw || static_cast< int >( output_texture_->get_height() ) != ph )
			{
				output_texture_ = gm->create_texture( core::graphics::PixelFormat::R8G8B8A8_UNORM, pw, ph );
			}
			gm->update_texture( output_texture_.get(), rgba_.data() );

			// 表示の間だけポストエフェクトを差し替え、描画後に元へ戻す（非NPRシーンに影響させない）
			core::graphics::Shader* prev_pe = gm->get_post_effect_shader();
			core::graphics::Shader* recolor_shader = gm->resolve_shader( recolor_shader_ );	// 解放されていれば nullptr（使わない）
			core::graphics::Shader* default_shader = gm->resolve_shader( default_shader_ );
			if ( color_preserve_ && recolor_shader )
			{
				recolor_shader->set_float( "blue", color_blue_ );
				recolor_shader->set_float( "yellow", color_yellow_ );
				recolor_shader->set_float( "a_spread", color_a_ );
				recolor_shader->set_float( "dark_clip", color_dark_ );
				recolor_shader->set_texture_at( 1, scene );  // 元シーン(色の基準)
				gm->set_post_effect_shader( recolor_shader );
			}
			else if ( default_shader )
			{
				gm->set_post_effect_shader( default_shader );  // 素通し（うねり無し）
			}
			gm->render_post_effect( output_texture_.get() );
			if ( prev_pe ) gm->set_post_effect_shader( prev_pe );

			const auto t4 = clock::now();
			ms_readback_ = msf( t0, t1 );
			ms_pre_      = msf( t1, t2 );   // ダウンサンプル + ノイズ生成
			ms_infer_    = msf( t2, t3 );
			ms_post_     = msf( t3, t4 );
			ms_total_    = msf( t0, t4 );
			( void ) t1b;
			displayed = true;
		}
		catch ( const std::exception& e )
		{
			enabled_ = false;
			debug_log( std::string( "[NeuralNPR] 推論に失敗したので NPR を止めます: " ) + e.what() );
		}
	}

	return displayed;
}

void NeuralNprEffect::render_imgui()
{
	ImGui::Separator();
	if ( is_loaded() )
	{
		ImGui::Checkbox( "Neural NPR (realtime, DirectML)", & enabled_ );
		ImGui::Checkbox( "  fp16 (off = fp32)", & use_fp16_ );
		ImGui::SliderInt( "  proc res (long side)", & proc_long_side_, 128, 1080 );
		ImGui::SliderFloat( "  boil (noise temporal)", & noise_temporal_, 0.05f, 1.0f );
		ImGui::Checkbox( "  color preserve (recolor)", & color_preserve_ );
		ImGui::Text( "  shaders: recolor=%s default=%s", ! recolor_shader_.is_null() ? "ok" : "NULL", ! default_shader_.is_null() ? "ok" : "NULL" );
		if ( color_preserve_ )
		{
			ImGui::SliderFloat( "  blue", & color_blue_, 0.f, 8.f );
			ImGui::SliderFloat( "  dark clip", & color_dark_, 0.f, 100.f );
		}
		ImGui::Text( "ms: read %.1f pre %.1f infer %.1f post %.1f / total %.1f", ms_readback_, ms_pre_, ms_infer_, ms_post_, ms_total_ );
	}
	else
	{
		ImGui::TextColored( ImVec4( 1.f, 0.5f, 0.2f, 1.f ), "Neural NPR: model not loaded" );
	}
}

} // namespace blue_sky::graphics
