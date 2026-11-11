#include "DevTools.h"

#include <blue_sky/DebugLog.h>

#include <game/MainLoop.h>

#include <common/string.h>

#include <windows.h>

#include <algorithm>
#include <cstdio>

namespace blue_sky::dev
{

DevTools::DevTools( const DevOptions& options, Hooks hooks )
	: options_( options )
	, hooks_( std::move( hooks ) )
{

}

/**
 * 開発用 : BLUE_SKY_DEBUG_FIXED_STEP_MSEC="ミリ秒" を設定すると、1 フレームの経過時間をいつもその値にする
 *
 * 本当の時間によらず、物理・紙の模様・線の揺れがフレームの数だけで決まる。画面を撮り比べるときに、exe の速さの違いで揺れないように
 */
void DevTools::apply_fixed_step( game::MainLoop& main_loop ) const
{
	if ( const unsigned long msec = options_.fixed_step_msec; msec > 0 )
	{
		main_loop.set_fixed_elapsed_msec( msec );
		debug_log( "[Debug] BLUE_SKY_DEBUG_FIXED_STEP_MSEC : 1 フレームの経過時間をいつも " + std::to_string( msec ) + " ミリ秒にします" );
	}
}

void DevTools::setup()
{
	// フレームの保存（BLUE_SKY_CAPTURE_FRAMES=60,120 と BLUE_SKY_CAPTURE_DIR（既定 dump））。
	// 自動操作の確認で、画面がロック中（ウィンドウを撮影できない）でも描画結果を確かめるために使う
	if ( ! options_.capture_frames.empty() )
	{
		capture_frames_ = options_.capture_frames;
		capture_directory_ = options_.capture_directory;
		CreateDirectoryA( capture_directory_.c_str(), nullptr );
	}

	// フレームの時間（BLUE_SKY_DEBUG_FRAME_TIME="最初のフレーム,最後のフレーム"（例 "300,1500"））。
	// その間のフレームについて、update の始まりから present の前まで（CPU の仕事の時間。垂直同期の待ちを含まない）と、
	// フレームの間隔を測り、最後のフレームで中央値などを debug_log に出す（ハンドルの作り替えの前後を比べるため。doc/ハンドルの設計.md）
	frame_time_first_ = options_.frame_time_first;
	frame_time_last_ = options_.frame_time_last;

	if ( frame_time_last_ > 0 )
	{
		frame_time_work_ms_.reserve( frame_time_last_ - frame_time_first_ + 1 );
		frame_time_interval_ms_.reserve( frame_time_last_ - frame_time_first_ + 1 );
		frame_time_present_ms_.reserve( frame_time_last_ - frame_time_first_ + 1 );
	}

	// シーンを順に替える（"stage_select,game_play:1-1,title" のように「シーン[:ステージ]」を , で並べる）。
	// SCENE_SEQUENCE_INTERVAL フレームごとに順にベースシーンを置き換える。シーンを替えて戻ったときにリソースが正しく引けるかを、入力を送らずに確かめるため
	scene_sequence_.assign( options_.scene_sequence.begin(), options_.scene_sequence.end() );
}

void DevTools::begin_frame()
{
	if ( frame_time_last_ > 0 )
	{
		LARGE_INTEGER now;
		QueryPerformanceCounter( & now );
		frame_time_previous_start_ = frame_time_start_;
		frame_time_start_ = now.QuadPart;
	}
}

void DevTools::update_scene_sequence( bool is_loading_frame )
{
	if ( ! scene_sequence_.empty() && ! is_loading_frame && ++scene_sequence_frames_ >= SCENE_SEQUENCE_INTERVAL )
	{
		scene_sequence_frames_ = 0;
		debug_log( "[Scene] 開発用の順でベースシーンを置き換えます: " + scene_sequence_.front().name + ( scene_sequence_.front().stage_name.empty() ? "" : " " + scene_sequence_.front().stage_name ) );

		if ( hooks_.request_scene )
		{
			hooks_.request_scene( scene_sequence_.front() );
		}

		scene_sequence_.pop_front();
	}
}

void DevTools::after_render()
{
	capture_frame_if_requested();
	record_frame_time();
}

void DevTools::capture_frame_if_requested()
{
	frame_count_++;

	// 開発用 : BLUE_SKY_DEBUG_DISPLAY_CHANGE（"幅x高さ"）を設定すると、120 フレーム目に解像度を変えて「適用」する
	// （排他フルスクリーン中の解像度の変更を、オプション画面を操作せずに確かめるため。設定ファイルを汚さないよう BLUE_SKY_SETTINGS_DIR と一緒に使う）
	if ( frame_count_ == DISPLAY_CHANGE_FRAME && options_.display_change_width > 0 && options_.display_change_height > 0 )
	{
		const int w = options_.display_change_width, h = options_.display_change_height;

		debug_log( "[Debug] 解像度を変えて適用します : " + std::to_string( w ) + "x" + std::to_string( h ) );

		if ( hooks_.change_display )
		{
			hooks_.change_display( w, h );
		}
	}

	if ( std::find( capture_frames_.begin(), capture_frames_.end(), frame_count_ ) == capture_frames_.end() )
	{
		return;
	}

	wchar_t name[ 32 ];
	swprintf_s( name, L"/capture_%04d.png", frame_count_ );	// 区切りは / （Windows でも使える）

	const bool ok = hooks_.save_back_buffer && hooks_.save_back_buffer( common::convert_to_wstring( capture_directory_ ) + name );

	// 文字のアトラスも保存する（BLUE_SKY_CAPTURE_GLYPH_ATLAS が設定されているとき。文字の描画の不具合を調べる）
	if ( options_.capture_glyph_atlas && hooks_.save_glyph_atlas )
	{
		swprintf_s( name, L"/atlas_%04d.png", frame_count_ );
		hooks_.save_glyph_atlas( common::convert_to_wstring( capture_directory_ ) + name );
	}

	debug_log( string_t( ok ? "フレームを保存しました: " : "フレームを保存できませんでした: " ) + capture_directory_ + " / " + std::to_string( frame_count_ ) );
}

void DevTools::record_frame_time()
{
	if ( frame_time_last_ <= 0 || frame_count_ < frame_time_first_ || frame_count_ > frame_time_last_ || frame_time_start_ == 0 )
	{
		return;
	}

	LARGE_INTEGER now, frequency;
	QueryPerformanceCounter( & now );
	QueryPerformanceFrequency( & frequency );

	const double to_ms = 1000.0 / static_cast< double >( frequency.QuadPart );
	frame_time_work_ms_.push_back( static_cast< double >( now.QuadPart - frame_time_start_ ) * to_ms );

	if ( frame_time_previous_start_ != 0 )
	{
		frame_time_interval_ms_.push_back( static_cast< double >( frame_time_start_ - frame_time_previous_start_ ) * to_ms );
	}

	if ( frame_count_ != frame_time_last_ )
	{
		return;
	}

	// 中央値・平均・95 パーセンタイル・最小・最大
	auto summary = [] ( std::vector< double > v )
	{
		if ( v.empty() )
		{
			return string_t( "なし" );
		}

		std::sort( v.begin(), v.end() );
		double sum = 0.0;

		for ( double x : v )
		{
			sum += x;
		}

		char text[ 160 ];
		sprintf_s( text, "中央値 %.3f ms / 平均 %.3f ms / 95%% %.3f ms / 最小 %.3f ms / 最大 %.3f ms",
			v[ v.size() / 2 ], sum / v.size(), v[ std::min( v.size() - 1, v.size() * 95 / 100 ) ], v.front(), v.back() );
		return string_t( text );
	};

	const string_t scene_name = hooks_.current_scene_name ? hooks_.current_scene_name() : string_t();

	debug_log( "[FrameTime] フレーム " + std::to_string( frame_time_first_ ) + "〜" + std::to_string( frame_time_last_ ) + "（" + std::to_string( frame_time_work_ms_.size() ) + " 枚）"
		+ " シーン " + ( scene_name.empty() ? string_t( "なし" ) : scene_name ) );
	debug_log( "[FrameTime] 仕事の時間（update の始まり〜present の前） : " + summary( frame_time_work_ms_ ) );
	debug_log( "[FrameTime] フレームの間隔 : " + summary( frame_time_interval_ms_ ) );
	debug_log( "[FrameTime] present の時間 : " + summary( frame_time_present_ms_ ) );
}

void DevTools::present( const std::function< void() >& present )
{
	// 開発用 : present にかかった時間も測る（BLUE_SKY_DEBUG_FRAME_TIME の範囲。最後のフレームの分はログの後なので入らない）
	const bool measure_present = frame_time_last_ > 0 && frame_count_ >= frame_time_first_ && frame_count_ < frame_time_last_;
	LARGE_INTEGER present_start = { }, present_end = { };

	if ( measure_present )
	{
		QueryPerformanceCounter( & present_start );
	}

	present();

	if ( measure_present )
	{
		LARGE_INTEGER frequency;
		QueryPerformanceCounter( & present_end );
		QueryPerformanceFrequency( & frequency );
		frame_time_present_ms_.push_back( static_cast< double >( present_end.QuadPart - present_start.QuadPart ) * 1000.0 / static_cast< double >( frequency.QuadPart ) );
	}
}

} // namespace blue_sky::dev
