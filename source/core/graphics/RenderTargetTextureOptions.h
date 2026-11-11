#pragma once

#include <type/type.h>

namespace core::graphics
{

/**
 * 描画先にできるテクスチャを作るときの指定（ミップの段数・MSAA）
 *
 * 既定の値は、指定できるようにする前と同じ（ミップ 1 段・画面と同じ MSAA）。
 */
struct RenderTargetTextureOptions
{
	/// 画面（バックバッファ）と同じ MSAA にする、を表す sample_count
	static constexpr uint_t SAMPLE_COUNT_SAME_AS_SCREEN = 0;

	uint_t mip_levels = 1;									///< ミップの段数（1 : ミップなし。2 以上は読むときに作る）
	uint_t sample_count = SAMPLE_COUNT_SAME_AS_SCREEN;		///< MSAA のサンプル数（0 : 画面と同じ。1 : MSAA なし）
	uint_t sample_quality = 0;								///< MSAA の品質（sample_count が 2 以上のときだけ使う）

	/**
	 * 作れる形に直した指定を返す
	 *
	 * - ミップの段数は 1 以上
	 * - MSAA のテクスチャはミップを持てないので、MSAA（sample_count が 0 か 2 以上）ならミップは 1 段
	 * - MSAA なし（sample_count 1）なら品質は 0
	 *
	 * @param screen_sample_count 画面の MSAA のサンプル数（sample_count が 0 のときに MSAA かどうかを決めるのに使う）
	 */
	RenderTargetTextureOptions normalized( uint_t screen_sample_count ) const
	{
		RenderTargetTextureOptions o = *this;

		if ( o.mip_levels < 1 )
		{
			o.mip_levels = 1;
		}

		const uint_t effective_sample_count = ( o.sample_count == SAMPLE_COUNT_SAME_AS_SCREEN ) ? screen_sample_count : o.sample_count;

		if ( effective_sample_count > 1 )
		{
			o.mip_levels = 1;
		}

		if ( o.sample_count == 1 )
		{
			o.sample_quality = 0;
		}

		return o;
	}
};

} // namespace core::graphics
