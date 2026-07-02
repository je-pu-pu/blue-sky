#pragma once

#include <vector>
#include <cstddef>
#include <cmath>

namespace blue_sky::graphics
{

/**
 * 空間白色・時間 EMA 平滑ノイズを 1 フレーム分生成する（std≈1 に正規化）。
 *
 * 事前計算した N(0,1) の table を off から読み（空間方向は白色）、前フレーム状態 prev と
 * 時間 EMA 平滑（係数 a）して out(size n) を埋める。EMA で減る分散を固定係数で std≈1 に戻す。
 * フレーム毎に off を変えると筆致が描き直され「ボイリング」になる（infer_stochastic.py 相当）。
 *
 * @param table 事前計算 N(0,1) 表。size は 2 の冪であること（& マスクで巡回するため）。
 * @param prev  EMA の前フレーム状態。size!=n なら 0 で初期化される。
 * @param out   出力（size = n。呼び出し側でリサイズ済みであること）。
 * @param off   table の読み出し開始オフセット（フレーム毎に変える）。
 * @param a     EMA 係数（0<a<=1）。小さいほど時間的に滑らか＝ボイリング弱。
 */
inline void step_coherent_noise(
	const std::vector< float >& table,
	std::vector< float >& prev,
	std::vector< float >& out,
	std::size_t off,
	float a )
{
	const std::size_t n = out.size();
	if ( prev.size() != n ) prev.assign( n, 0.f );
	if ( table.empty() ) return;

	const std::size_t mask = table.size() - 1;             // table.size() は 2 の冪
	const float scale = std::sqrt( ( 2.f - a ) / a );      // EMA で減った分散を std≈1 に戻す
	const float* t = table.data();
	for ( std::size_t i = 0; i < n; ++i )
	{
		const float white = t[ ( off + i ) & mask ];
		const float v = ( 1.f - a ) * prev[ i ] + a * white;
		prev[ i ] = v;
		out[ i ] = v * scale;
	}
}

} // namespace blue_sky::graphics
