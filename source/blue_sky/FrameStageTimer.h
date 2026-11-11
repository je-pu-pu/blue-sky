#pragma once

#include <blue_sky/DebugLog.h>

#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

#include <windows.h>

namespace blue_sky
{

/**
 * 開発用 : 1 フレームの中の段階ごとの時間（CPU）と、描いた回数（ドローコール）を測り、中央値を debug_log に出す
 *
 * 環境変数 BLUE_SKY_DEBUG_FRAME_STAGES="最初のフレーム,最後のフレーム"（例 "200,800"）を設定したときだけ動く。設定しないと何もしない
 * （mark などは 1 回の bool の比較だけ）。フレームの番号は、測るシーンが begin_frame を呼んだ回数（シーンが始まってから数える）。
 *
 * 使い方 : フレームの初めに begin_frame、段階の終わりごとに mark( 名前 )（前の mark からの時間をその名前に足す）、
 * フレームの終わりに end_frame。同じ名前を 1 フレームに何度 mark してもよい（足し合わせる）。
 *
 * D3D11 は描く命令を溜めて後で GPU に送るので、描く段階の時間は「命令を作る CPU の時間」。GPU の時間は Present の待ちに出る
 * （GameMain の BLUE_SKY_DEBUG_FRAME_TIME の「Present の時間」）。
 */
class FrameStageTimer
{
private:
	struct Stage
	{
		std::string				name;
		double					current = 0.0;	///< このフレームの合計（ms）
		std::vector< double >	samples;		///< フレームごとの合計（ms）
	};

	bool					enabled_ = false;
	int						first_ = 0;
	int						last_ = 0;
	int						frame_ = 0;
	bool					in_range_ = false;
	LARGE_INTEGER			previous_ = { };
	LARGE_INTEGER			frame_start_ = { };
	double					to_ms_ = 0.0;
	std::vector< Stage >	stages_;
	std::vector< double >	total_samples_;
	std::vector< double >	draw_samples_;

	Stage& find_or_add( const char* name )
	{
		for ( auto& s : stages_ )
		{
			if ( s.name == name )
			{
				return s;
			}
		}

		stages_.push_back( { name } );
		return stages_.back();
	}

	static double median( std::vector< double > v )
	{
		if ( v.empty() )
		{
			return 0.0;
		}

		std::nth_element( v.begin(), v.begin() + v.size() / 2, v.end() );
		return v[ v.size() / 2 ];
	}

public:
	FrameStageTimer()
	{
		char buffer[ 64 ] = { };

		if ( GetEnvironmentVariableA( "BLUE_SKY_DEBUG_FRAME_STAGES", buffer, sizeof( buffer ) ) == 0 || sscanf_s( buffer, "%d,%d", & first_, & last_ ) != 2 || last_ < first_ )
		{
			return;
		}

		LARGE_INTEGER frequency;
		QueryPerformanceFrequency( & frequency );
		to_ms_ = 1000.0 / static_cast< double >( frequency.QuadPart );
		enabled_ = true;
	}

	bool is_enabled() const { return enabled_; }

	void begin_frame()
	{
		if ( ! enabled_ )
		{
			return;
		}

		++frame_;
		in_range_ = frame_ >= first_ && frame_ <= last_;

		if ( in_range_ )
		{
			for ( auto& s : stages_ )
			{
				s.current = 0.0;
			}

			QueryPerformanceCounter( & previous_ );
			frame_start_ = previous_;
		}
	}

	void mark( const char* name )
	{
		if ( ! in_range_ )
		{
			return;
		}

		LARGE_INTEGER now;
		QueryPerformanceCounter( & now );
		find_or_add( name ).current += static_cast< double >( now.QuadPart - previous_.QuadPart ) * to_ms_;
		previous_ = now;
	}

	/// @param draw_count このフレームで描いた回数（GraphicsManager::get_draw_count）
	/// @return 測る範囲の最後のフレームで、結果をログに出したら true（呼ぶ側が続けて自分の数を出せる）
	bool end_frame( unsigned draw_count )
	{
		if ( ! in_range_ )
		{
			return false;
		}

		LARGE_INTEGER now;
		QueryPerformanceCounter( & now );

		for ( auto& s : stages_ )
		{
			s.samples.push_back( s.current );
		}

		total_samples_.push_back( static_cast< double >( now.QuadPart - frame_start_.QuadPart ) * to_ms_ );
		draw_samples_.push_back( static_cast< double >( draw_count ) );
		in_range_ = false;

		if ( frame_ != last_ )
		{
			return false;
		}

		char text[ 256 ];
		sprintf_s( text, "[FrameStage] フレーム %d〜%d（%zu 枚）の中央値 : 合計 %.3f ms・描いた回数 %.0f", first_, last_, total_samples_.size(), median( total_samples_ ), median( draw_samples_ ) );
		debug_log( text );

		for ( const auto& s : stages_ )
		{
			// 途中で増えた段階は、増える前のフレームの分が無い（そのまま出す）
			sprintf_s( text, "[FrameStage]   %s : %.3f ms", s.name.c_str(), median( s.samples ) );
			debug_log( text );
		}

		return true;
	}
};

} // namespace blue_sky
