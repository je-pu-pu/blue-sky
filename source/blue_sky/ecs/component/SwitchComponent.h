#pragma once

#include <core/ecs/Component.h>
#include <core/type.h>

#include <functional>
#include <list>
#include <unordered_map>

namespace blue_sky::ecs
{

/**
 * スイッチ（入・切・壊れた の状態と、入・切になったときのイベントの受け取り手）
 *
 * 以前の BaseSwitch・Switch・AreaSwitch クラスの代わり。状態を変えるのは SwitchSystem（音・アニメーションも鳴らす）。
 * 触れた物体の間引き（0.5 秒）は ContactCacheComponent が持つ。
 *
 * - 通常のスイッチ（"switch"）: 硬い箱。プレイヤーと速く飛ぶ石で入・切が変わり、アニメーションと音がある
 * - 空間スイッチ（ステージの area-switch 行）: すり抜ける箱。プレイヤーだけで変わり、アニメーションと音は無い
 */
struct SwitchComponent : public core::ecs::Component
{
	enum class State { OFF = 0, ON, BROKEN };

	using EventHandler		= std::function< void() >;
	using EventHandlerList	= std::list< EventHandler >;
	using EventHandlerMap	= std::unordered_map< string_t, EventHandlerList >;

	State			state = State::OFF;

	bool			is_switched_by_stone = true;	///< 石で入・切が変わるか（空間スイッチは変わらない。以前の AreaSwitch::on_collide_with( Stone* ) が空）
	bool			has_feedback = true;			///< 入・切・壊れたでアニメーションと音を鳴らすか（以前の Switch の on_turn_on など。空間スイッチは鳴らさない）

	EventHandlerMap	event_handler_map;				///< イベントの名前（"turn-on"・"turn-off"）→ 受け取り手（ステージの event 行。加えた順に呼ぶ）
};

} // namespace blue_sky::ecs
