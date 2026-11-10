#pragma once

#include <core/type.h>

class btRigidBody;

namespace blue_sky::ecs::object_body
{

/**
 * 本編の物理の世界の剛体を操作する（ActiveObject と ECS だけの物体で同じ中身を使う）
 *
 * 以前は GameObject のメンバ関数だけが持っていた。ECS だけの物体（EntityObject）も同じ決まりで動かすため、ここへ移した。
 * GameObject の同じ名前の関数は、これを呼ぶ
 */

/// 硬い物のやり直したときの重力（地球。以前の GameObject::GravityDefault）
extern const core::Vector GravityDefault;

/// location にあって velocity で動く物が、target_location に近づいているか（以前の GameObject::is_moving_to。敵が石に当たったときに使う）
bool is_moving_to( const core::Vector& location, const core::Vector& velocity, const core::Vector& target_location );

/// 速度（剛体の線形速度）
const core::Vector& get_velocity( const btRigidBody& body );

/// 速度を設定する（同じ速度なら何もしない。変えるときは補間用の速度もそろえ、剛体を起こす）
void set_velocity( btRigidBody& body, const core::Vector& v );

/// 質量を設定する（形から慣性を計算し直す）
void set_mass( btRigidBody& body, float_t mass );

void set_gravity( btRigidBody& body, const core::Vector& gravity );
void set_friction( btRigidBody& body, float_t friction );
void set_angular_factor( btRigidBody& body, float_t factor );
void set_angular_factor( btRigidBody& body, const core::Vector& factor );
void set_kinematic( btRigidBody& body, bool is_kinematic );
void set_no_contact_response( btRigidBody& body, bool is_no_contact_response );

} // namespace blue_sky::ecs::object_body
