#pragma once

#include <core/type.h>

#include <cmath>
#include <random>

namespace blue_sky::ecs::flicker
{

/**
 * ゆらぎ（ふわふわと上下する動き）と点滅の計算（ActiveObject と ECS だけの物体で同じ中身を使う）
 *
 * 以前は GameObject のメンバ関数（get_flicker_height_offset・is_visible_in_blink）だけが持っていた。
 * 風船・少女・敵を ECS だけの物体にしても同じ値になるよう、ここへ移した。GameObject の同じ名前の関数は、これを呼ぶ
 */

/**
 * ゆらぎによる高さのずれ
 *
 * 物体ごとに決まった位相を、物体ごとに決まった種（Entity の番号）から作る。
 * 自分だけの乱数を使い、共有の乱数には触らない（同じ種なら同じ値）
 *
 * @param seed 種（物体の Entity の番号）
 * @param scene_elapsed_time シーンが始まってからの秒
 * @param scale ゆらぎの大きさ
 */
inline float_t height_offset( unsigned int seed, float_t scene_elapsed_time, float_t scale )
{
	std::mt19937 engine( seed );
	const float_t a = std::uniform_real_distribution< float_t >( 0.f, 10.f )( engine );

	return std::cos( a + scene_elapsed_time ) * scale * 0.5f;
}

/**
 * 点滅しているときに、今は見えるか
 *
 * @param scene_elapsed_time シーンが始まってからの秒
 * @param blink_count 1 秒の間に点滅する回数
 */
inline bool is_visible_in_blink( float_t scene_elapsed_time, float_t blink_count )
{
	return std::fmodf( scene_elapsed_time, 1.f / blink_count ) < ( 0.5f / blink_count );
}

} // namespace blue_sky::ecs::flicker
