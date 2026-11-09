#pragma once

#include <blue_sky/type.h>

#include <functional>

class btDynamicsWorld;

namespace core::sound { class SoundManager; }

namespace blue_sky
{

/**
 * ゲームのオブジェクトが使うもの（物理の世界・音・シーンの経過時間）
 *
 * 組み立ての根（GameMain）が作り、ActiveObjectManager がオブジェクトを加えるときに渡す。
 * 以前は GameObject が GameMain のシングルトンから直接取っていて、テストで差し替えられなかった（設計上の問題 B28）。
 */
struct GameObjectContext
{
	std::function< btDynamicsWorld*() >		dynamics_world;			///< 物理の世界
	core::sound::SoundManager*				sound_manager = nullptr;	///< 音
	std::function< float_t() >				scene_elapsed_time;		///< 今のシーンが始まってからの秒数
};

} // namespace blue_sky
