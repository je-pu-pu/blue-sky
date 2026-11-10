#pragma once

#include <memory>

class btDynamicsWorld;
class btRigidBody;
class btMotionState;
class btCollisionShape;

namespace core::physics
{

/**
 * 剛体の持ち主（ムーブだけできる）
 *
 * 壊れるとき（reset・ムーブで上書きされたときも）に、剛体を世界から外し、剛体・動きの状態・（持っていれば）形を壊す。
 * 部品（core::ecs::RigidBodyComponent・blue_sky::ecs::PhysicsBodyComponent）が値で持ち、部品が外れる・Entity が消えると剛体も消える。
 *
 * 以前は剛体を生の new で作り、持ち主がはっきりしなかった（PhysicsManager の剛体は PhysicsSystem が destroy_rigid_body を呼んで消し、
 * 本編の剛体は物理の世界が持ち、物体を壊しても世界に残ってシーンの終わりにまとめて消していた）。
 *
 * 世界（btDynamicsWorld）はこれより長く生きること。形は、共有するもの（BulletPhysics が大きさごとに使い回す）は持たず、
 * この剛体だけのもの（PhysicsManager が剛体ごとに作る）は持つ
 */
class RigidBody
{
private:
	btDynamicsWorld*					world_ = nullptr;		///< 剛体を入れた世界（無ければ nullptr。外すときに使う）
	std::unique_ptr< btRigidBody >		rigid_body_;
	std::unique_ptr< btMotionState >	motion_state_;
	std::unique_ptr< btCollisionShape >	owned_shape_;			///< この剛体だけの形（共有する形なら nullptr）

public:
	RigidBody();

	/**
	 * @param world 剛体を入れた世界（壊すときにここから外す。nullptr なら外さない）
	 * @param rigid_body 剛体（既に world に入れてあること）
	 * @param motion_state 剛体の動きの状態
	 * @param owned_shape この剛体だけの形（共有する形なら nullptr）
	 */
	RigidBody( btDynamicsWorld* world, std::unique_ptr< btRigidBody > rigid_body, std::unique_ptr< btMotionState > motion_state, std::unique_ptr< btCollisionShape > owned_shape = nullptr );

	~RigidBody();

	RigidBody( RigidBody&& ) noexcept;
	RigidBody& operator = ( RigidBody&& ) noexcept;

	RigidBody( const RigidBody& ) = delete;
	RigidBody& operator = ( const RigidBody& ) = delete;

	/// 世界から外して壊す（持っていなければ何もしない）
	void reset();

	bool is_valid() const { return static_cast< bool >( rigid_body_ ); }
	explicit operator bool () const { return is_valid(); }

	/// 剛体（持っていなければ nullptr。ポインタを持ち続けるなら、この持ち主より長く使わないこと）
	btRigidBody* get() const { return rigid_body_.get(); }
	btRigidBody* operator -> () const { return rigid_body_.get(); }

}; // class RigidBody

} // namespace core::physics
