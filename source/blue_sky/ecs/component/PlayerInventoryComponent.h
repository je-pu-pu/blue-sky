#pragma once

#include <blue_sky/GameObjectRef.h>
#include <core/ecs/Component.h>

#include <list>

namespace blue_sky::ecs
{

/**
 * プレイヤーの持ち物（ロケット・傘・石・望遠鏡の数、選んでいる持ち物、持っている石、メダル）
 *
 * 以前は Player のメンバ変数で、段階 6 で一度 PlayerBehaviorComponent にまとめてから切り出した。
 * 拾う・選ぶ・使う（減らす）は PlayerInventorySystem。行動（PlayerBehaviorSystem）は、ロケットで飛ぶ・石を投げるときに減らすよう頼み、
 * 走れるか（石が 3 個未満）・望遠鏡をやめる（選ぶのを戻す）ときに読み書きする
 */
struct PlayerInventoryComponent : public core::ecs::Component
{
	enum class ItemType
	{
		NONE = -1,
		ROCKET = 0,
		UMBRELLA,
		STONE,
		SCOPE,
		MAX
	};

	using StoneList = std::list< GameObjectRef >;	///< 持っている石（EntityId を包む窓口）

	int			item_count[ static_cast< int >( ItemType::MAX ) ] = { 0 };	///< 各アイテムの個数
	ItemType	selected_item_type = ItemType::NONE;	///< 現在選択中のアイテム
	StoneList	stone_list;								///< 現在持っている石のリスト
	bool		has_medal = false;						///< メダル保持フラグ

	int get_item_count( ItemType type ) const
	{
		if ( type <= ItemType::NONE ) return 0;
		if ( type >= ItemType::MAX ) return 0;

		return item_count[ static_cast< int >( type ) ];
	}
};

} // namespace blue_sky::ecs
