#pragma once

#include <array>

namespace blue_sky
{

/**
 * ウィンドウメッセージで届いた「押した」を、ポーリングの結果に足す
 *
 * キーの状態は毎フレーム GetAsyncKeyState で読んでいるため、1 フレーム（約 16 ミリ秒）の間に押して離した
 * 短いタップは取りこぼしていた（設計上の問題 B27）。WM_KEYDOWN などで届いた押下を溜めておき、
 * ポーリングで押されていない・前のフレームも押されていなかったキーは、このフレームだけ押されたことにする。
 * 溜めた押下は使ったら消す。
 *
 * @param now 今のフレームの押下状態（ポーリングの結果。足し込む）
 * @param prev 前のフレームの押下状態
 * @param latched ウィンドウメッセージで届いた押下（消す）
 */
inline void apply_latched_presses( std::array< bool, 256 >& now, const std::array< bool, 256 >& prev, std::array< bool, 256 >& latched )
{
	for ( size_t vk = 0; vk < now.size(); vk++ )
	{
		if ( latched[ vk ] && ! now[ vk ] && ! prev[ vk ] )
		{
			now[ vk ] = true;
		}
	}

	latched.fill( false );
}

} // namespace blue_sky
