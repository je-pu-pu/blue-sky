#pragma once

#include <cstddef>
#include <vector>

namespace blue_sky
{

/// オーバーレイ 1 枚の描画に関する性質（Scene::is_transparent / hides_overlays_below の値）
struct OverlayLayerInfo
{
	bool transparent;			///< 下のシーンを見せる
	bool hides_overlays_below;	///< 下に積まれたオーバーレイを隠す
};

/// 描画する範囲
struct OverlayRenderRange
{
	size_t first_overlay = 0;	///< この添字から上のオーバーレイを描く
	bool render_base = true;	///< ベースシーンを描く
};

/**
 * オーバーレイの積み重ね（下から順）から、描画する範囲を決める
 *
 * - 上から見て最初の「下のオーバーレイを隠す」または「不透明」なものより下のオーバーレイは描かない。
 * - 不透明なオーバーレイが 1 枚でもあれば（隠されていても）、ベースシーンは描かない
 *   （隠されたオーバーレイも、ベースシーンを覆っていることに変わりはないため）。
 */
inline OverlayRenderRange compute_overlay_render_range( const std::vector< OverlayLayerInfo >& layers )
{
	OverlayRenderRange range;
	bool first_decided = false;

	for ( size_t i = layers.size(); i > 0; i-- )
	{
		const auto& layer = layers[ i - 1 ];

		if ( ! layer.transparent )
		{
			if ( ! first_decided )
			{
				range.first_overlay = i - 1;
			}

			range.render_base = false;
			return range;
		}

		if ( ! first_decided && layer.hides_overlays_below )
		{
			range.first_overlay = i - 1;
			first_decided = true;
		}
	}

	return range;
}

} // namespace blue_sky
