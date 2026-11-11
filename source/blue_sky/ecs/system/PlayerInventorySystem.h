#pragma once

#include <core/ecs/System.h>
#include <blue_sky/ecs/component/PlayerInventoryComponent.h>

#include <optional>

namespace core::sound { class SoundManager; }

namespace blue_sky::ecs
{

enum class PickupKind;

/**
 * プレイヤーの持ち物（PlayerInventoryComponent）を拾う・選ぶ・使う（以前の Player の同じ名前の関数。値と順は以前と同じ）
 *
 * 音はプレイヤーの位置で鳴らす（拾った音・選んだ音）。EntityManager::update() では何もしない（呼ばれたときだけ動く）
 */
class PlayerInventorySystem : public core::ecs::System< PlayerInventoryComponent >
{
public:
	using ItemType = PlayerInventoryComponent::ItemType;

private:
	core::sound::SoundManager*	sound_manager_;		///< 音を鳴らす先（nullptr なら鳴らさない。テスト）

	void play_sound( core::ecs::EntityId id, const char_t* name ) const;

public:
	explicit PlayerInventorySystem( core::sound::SoundManager* sound_manager ) : sound_manager_( sound_manager ) { }

	void update() override { }		///< 何もしない（上の説明のとおり）

	/// 持ち物（プレイヤーでなければ nullptr）
	PlayerInventoryComponent* find_inventory( core::ecs::EntityId id ) const;

	/// やり直す（持ち物を空にし、望遠鏡を 1 つ持つ。以前の Player::restart の持ち物の所）
	void restart( core::ecs::EntityId id );

	/// 石を拾う（いつも true。以前の Player::on_collide_with_stone）
	bool pick_stone( core::ecs::EntityId id, const GameObjectRef& stone );

	/// メダル・ロケットを拾う（いつも true。傘は持ち物にならない。以前の Player::on_pick_up）
	bool pick_up( core::ecs::EntityId id, PickupKind kind );

	void select_prev_item( core::ecs::EntityId id );
	void select_next_item( core::ecs::EntityId id );

	/// ロケットを 1 つ使う（無ければ false。無くなったら選ぶのを戻す）
	bool use_rocket( core::ecs::EntityId id );

	/// 最後に拾った石を 1 つ使う（無ければ無い。無くなったら選ぶのを戻す）
	std::optional< GameObjectRef > use_stone( core::ecs::EntityId id );
};

} // namespace blue_sky::ecs
