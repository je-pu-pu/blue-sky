#pragma once

#include <d3d11.h>
#include <vector>
#include <cstdint>

namespace core::graphics::direct_3d_11
{

/**
 * R8G8B8A8 系テクスチャを CPU 側の連続 RGBA8 バッファへ読み戻す。
 *
 * MSAA テクスチャは非 MSAA へ解決してから読む。行ピッチは詰めて連続化する。
 * ( フレームダンプ・ニューラル推論入力など、GPU→CPU 読み戻しの共通処理 )
 *
 * @param out_pixels 読み戻し先。size = width*height*4 にリサイズされる
 * @return 成功したら true
 */
bool read_texture_2d_to_rgba8(
	ID3D11Device* device,
	ID3D11DeviceContext* context,
	ID3D11Texture2D* source,
	std::vector< std::uint8_t >& out_pixels,
	UINT& out_width,
	UINT& out_height );

/**
 * 8 ビット RGBA / BGRA のテクスチャを PNG として保存する（WIC を使う。COM は WinMain で初期化済み）
 *
 * フレームダンプ・開発用の画面の保存（画面がロック中でも描画結果を確かめられる）に使う。
 *
 * @return 成功したら true
 */
bool save_texture_2d_to_png(
	ID3D11Device* device,
	ID3D11DeviceContext* context,
	ID3D11Texture2D* source,
	const wchar_t* file_name );

} // namespace core::graphics::direct_3d_11
