#pragma once

#include <blue_sky/type.h>

#include <functional>


namespace core::sound { class SoundManager; }

namespace blue_sky
{

class ActiveObjectPhysics;

/**
 * ゲームのオブジェクトが使うもの（物理の世界・音・シーンの経過時間）
 *
 * 組み立ての根（GameMain）が作り、ObjectUpdater に渡す（System に経過時間・音を渡すのに使う）。
 * 以前は GameObject が GameMain のシングルトンから直接取っていて、テストで差し替えられなかった（設計上の問題 B28）。
 * 段階 6 で GameObject・ActiveObject・ActiveObjectManager を消した。ActiveObject を知らない（物理の世界の型を前方宣言するだけ）
 */
struct GameObjectContext
{
	const ActiveObjectPhysics*				physics = nullptr;		///< 物理（当たりの問い合わせに使う。以前は Bullet の世界そのものを渡していた）
	core::sound::SoundManager*				sound_manager = nullptr;	///< 音
	std::function< float_t() >				scene_elapsed_time;		///< 今のシーンが始まってからの秒数
	std::function< float_t() >				delta_time;				///< 前のフレームからの秒数（以前は core::Service の TimeManager から取っていた）
};

} // namespace blue_sky
