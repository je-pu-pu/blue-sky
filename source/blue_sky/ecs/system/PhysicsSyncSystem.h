#pragma once

#include <core/ecs/System.h>
#include <core/ecs/component/TransformComponent.h>
#include <blue_sky/ecs/component/PhysicsBodyComponent.h>

class btRigidBody;

namespace blue_sky::ecs
{

/**
 * 本編の物理の世界の剛体と、位置の正（TransformComponent）を同期する
 *
 * - update_transforms : 物理を進めた後に、剛体の位置と回転を TransformComponent へ写す
 *   （以前は GamePlayScene が全物体の GameObject::update_transform を呼んでいた）
 * - write_body : 位置を変えたとき（set_location・向きを変える・やり直す）に、TransformComponent の位置を剛体へ送る
 *   （GameObject::commit_transform が呼ぶ。以前は GameObject が自分で計算していた）
 *
 * 剛体の中心は物体の位置（足元）から height_offset だけ上にある（以前と同じ計算）。
 * EntityManager::update() では何もしない（物理を進める時点は本編のシーンが決める）
 */
class PhysicsSyncSystem : public core::ecs::System< core::ecs::TransformComponent, PhysicsBodyComponent >
{
public:
	PhysicsSyncSystem() = default;

	void update() override { }		///< 何もしない（上の説明のとおり）

	/// 全部の剛体の位置と回転を TransformComponent へ写す（物理を進めた後に毎フレーム）
	void update_transforms();

	/// 剛体の位置と回転から、物体の位置（足元）と回転を返す（以前の GameObject::update_transform の計算）
	static core::Transform read_body( const btRigidBody& body, float_t height_offset );

	/// 物体の位置（足元）と回転を剛体へ送る（以前の GameObject::commit_transform の計算。補間用の位置も同じにする）
	static void write_body( btRigidBody& body, float_t height_offset, const core::Transform& transform );
};

} // namespace blue_sky::ecs
