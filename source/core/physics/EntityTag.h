#pragma once

#include <core/ecs/type.h>

#include <BulletCollision/CollisionDispatch/btCollisionObject.h>

#include <cstdint>
#include <optional>

namespace core::physics
{

/**
 * 剛体の userPointer に入れる「Entity の番号のタグ」
 *
 * 以前は userPointer に物体のポインタ（本編は ActiveObject*、ECS は RigidBodyComponent*）を入れ、
 * 当たり・レイの判定で reinterpret_cast して呼んでいた。物体が消えた後も剛体が残ると、消えた物を指し得た。
 * 今は Entity の番号を入れ、使う側が EntityManager で引く（消えていれば引けない。EntityId は二度と使わない番号なので、
 * 別の物を指すことも無い）。
 *
 * 番号 + 1 を入れる（0 番の Entity と「タグ無し」= nullptr を区別するため）。タグの無い剛体（地面・ステージの当たりの形）は nullptr のまま
 */
inline void* make_entity_tag( ecs::EntityId id )
{
	return reinterpret_cast< void* >( static_cast< std::uintptr_t >( id ) + 1u );
}

/// userPointer からタグを読む（タグが無ければ std::nullopt）
inline std::optional< ecs::EntityId > read_entity_tag( const void* user_pointer )
{
	if ( ! user_pointer )
	{
		return std::nullopt;
	}

	return static_cast< ecs::EntityId >( reinterpret_cast< std::uintptr_t >( user_pointer ) - 1u );
}

/// 剛体（当たりの物）に Entity の番号のタグを付ける
inline void set_entity_tag( btCollisionObject* object, ecs::EntityId id )
{
	object->setUserPointer( make_entity_tag( id ) );
}

/// 剛体のタグを外す（地面と同じ「タグ無し」になる）
inline void clear_entity_tag( btCollisionObject* object )
{
	object->setUserPointer( nullptr );
}

/// 剛体のタグを読む（タグが無い・object が nullptr なら std::nullopt）
inline std::optional< ecs::EntityId > get_entity_tag( const btCollisionObject* object )
{
	return object ? read_entity_tag( object->getUserPointer() ) : std::nullopt;
}

} // namespace core::physics
