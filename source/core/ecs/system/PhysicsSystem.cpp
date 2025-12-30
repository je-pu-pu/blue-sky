#include "PhysicsSystem.h"
#include <core/physics/PhysicsManager.h>
#include <core/math/Quaternion.h>
#include <btBulletDynamicsCommon.h>

namespace core::ecs
{

using PhysicsManager = core::physics::PhysicsManager;
using RigidBodyCreateInfo = core::physics::RigidBodyCreateInfo;
using Quaternion = core::math::Quaternion;
using Vector = core::math::Vector;

void PhysicsSystem::initialize_rigid_body( RigidBodyComponent* rigid_body, TransformComponent* transform )
{
	if ( rigid_body->handle.is_valid() )
	{
		return;
	}

	auto* physics_manager = PhysicsManager::get_instance();

	if ( ! physics_manager )
	{
		return;
	}

	// 生成パラメータを設定
	RigidBodyCreateInfo info;
	info.shape_type = rigid_body->shape_type;
	info.shape_size = rigid_body->shape_size;
	info.mass = rigid_body->mass;
	info.collision_group = rigid_body->collision_group;
	info.collision_mask = rigid_body->collision_mask;

	// offset を適用した transform を設定
	info.transform = transform->transform;
	Vector rotated_offset = Vector::transform( rigid_body->offset, transform->transform.get_rotation() );
	info.transform.get_position() += rotated_offset;

	// 剛体を生成
	rigid_body->handle = physics_manager->create_rigid_body( info );

	// 角速度係数と摩擦係数を設定
	if ( rigid_body->handle.is_valid() )
	{
		physics_manager->set_rigid_body_angular_factor( rigid_body->handle, rigid_body->angular_factor );
		rigid_body->handle.rigid_body->setFriction( rigid_body->friction );

		// user_pointer に RigidBodyComponent へのポインタを設定
		rigid_body->handle.user_pointer = rigid_body;
		rigid_body->handle.rigid_body->setUserPointer( rigid_body );
	}
}

void PhysicsSystem::update()
{
	auto* physics_manager = PhysicsManager::get_instance();

	if ( ! physics_manager )
	{
		return;
	}

	// 物理シミュレーションのステップを進める
	physics_manager->step_simulation();

	// 各 Entity の Component を更新
	for ( auto& pair : get_component_list() )
	{
		auto& component_tuple = pair.second;

		auto* rigid_body = std::get<RigidBodyComponent*>( component_tuple );
		auto* transform = std::get<TransformComponent*>( component_tuple );

		// 未初期化の RigidBody を初期化
		if ( ! rigid_body->handle.is_valid() )
		{
			initialize_rigid_body( rigid_body, transform );
		}

		// 物理演算の結果を TransformComponent に反映
		if ( rigid_body->handle.is_valid() )
		{
			Vector position;
			Quaternion rotation;

			physics_manager->get_rigid_body_transform( rigid_body->handle, position, rotation );

			// offset を引いて Entity の位置を算出
			Vector rotated_offset = Vector::transform( rigid_body->offset, rotation );
			transform->transform.set_position( position - rotated_offset );
			transform->transform.set_rotation( rotation );
		}
	}
}

void PhysicsSystem::update( ComponentTuple& ) const
{
	// update() でまとめて処理するため、ここでは何もしない
}

} // namespace core::ecs
