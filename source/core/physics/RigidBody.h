#pragma once

#include <core/math/Vector.h>
#include <core/math/Transform.h>
#include <core/ecs/type.h>
#include <type/type.h>

#include <memory>
#include <optional>

class btDynamicsWorld;
class btRigidBody;
class btMotionState;
class btCollisionShape;

namespace core::physics
{

class PhysicsManager;

/**
 * 剛体の形状タイプ
 */
enum class RigidBodyShapeType
{
	Box,
	Capsule,
	Cylinder,
	Sphere,
	TriangleMesh,	///< ステージの当たりの形（PhysicsManager::load_obj が作る）
	Other,
};

/**
 * 剛体の持ち主（ムーブだけできる）
 *
 * 壊れるとき（reset・ムーブで上書きされたときも）に、剛体を世界から外し、剛体・動きの状態・（持っていれば）形を壊す。
 * 部品（core::ecs::RigidBodyComponent・blue_sky::ecs::PhysicsBodyComponent）が値で持ち、部品が外れる・Entity が消えると剛体も消える。
 *
 * 以前は剛体を生の new で作り、持ち主がはっきりしなかった（PhysicsManager の剛体は PhysicsSystem が destroy_rigid_body を呼んで消し、
 * 本編の剛体は物理の世界が持ち、物体を壊しても世界に残ってシーンの終わりにまとめて消していた）。
 *
 * 剛体の操作（速度・質量・重力・当たりの応答など）はこのクラスのメンバ関数で行う。Bullet の剛体（btRigidBody）は
 * core/physics の外には出さない（以前は blue_sky が btRigidBody* を持って直接呼んでいた）。
 *
 * 世界（btDynamicsWorld）はこれより長く生きること。形は、共有するもの（PhysicsManager が大きさごとに使い回す）は持たず、
 * この剛体だけのものは持つ
 */
class RigidBody
{
public:
	using Vector = math::Vector;
	using Transform = math::Transform;

private:
	btDynamicsWorld*					world_ = nullptr;		///< 剛体を入れた世界（無ければ nullptr。外すときに使う）
	std::unique_ptr< btRigidBody >		rigid_body_;
	std::unique_ptr< btMotionState >	motion_state_;
	std::unique_ptr< btCollisionShape >	owned_shape_;			///< この剛体だけの形（共有する形なら nullptr）

	friend class PhysicsManager;

	/// Bullet の剛体（core/physics の中だけで使う。持っていなければ nullptr。以前は public で、本編が Bullet の剛体を直接触っていた）
	btRigidBody* get() const { return rigid_body_.get(); }

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

	/// 同じ剛体か（当たり・レイで自分を見分ける）
	bool is_same( const RigidBody& other ) const { return rigid_body_ && rigid_body_ == other.rigid_body_; }

	// 以下、剛体を持っていないときに呼んではいけない（is_valid を確かめてから呼ぶ）

	// ---- 位置 ----

	/// 世界の位置と回転に、剛体から見たずれ local_offset を掛けたもの（以前の PhysicsSyncSystem::read_body の Bullet の計算と同じ）
	Transform get_world_transform( const Vector& local_offset ) const;

	/// transform に剛体から見たずれ local_offset を掛けて、世界の位置と補間用の位置にする（以前の PhysicsSyncSystem::write_body）
	void set_world_transform( const Transform& transform, const Vector& local_offset );

	/// 世界の位置と補間用の位置をそのまま transform にする（ずれは掛けない）
	void set_world_transform( const Transform& transform );

	/// 動きの状態（描画などに補間して渡す位置）の位置を transform にする
	void set_motion_state_transform( const Transform& transform );

	/// 重心の位置
	Vector get_center_of_mass_position() const;

	// ---- 速度・力 ----

	Vector get_linear_velocity() const;
	void set_linear_velocity( const Vector& );
	void set_interpolation_linear_velocity( const Vector& );
	void set_angular_velocity( const Vector& );
	void set_interpolation_angular_velocity( const Vector& );
	void clear_forces();

	// ---- 眠り ----

	/// 起こす（Bullet の activate）
	void activate( bool force_activation = false );

	/// 眠りの状態を「動いている」（ACTIVE_TAG）にする（以前の setActivationState( true )。true は 1 = ACTIVE_TAG）
	void set_activation_state_active();

	// ---- 性質 ----

	/// 質量を設定する（形から慣性を計算し直す）
	void set_mass( float_t mass );
	float_t get_inv_mass() const;

	void set_gravity( const Vector& gravity );
	Vector get_gravity() const;

	void set_friction( float_t friction );
	float_t get_friction() const;

	void set_angular_factor( float_t factor );
	void set_angular_factor( const Vector& factor );
	Vector get_angular_factor() const;

	/// 動きを物理に任せず位置で動かす物にする（CF_KINEMATIC_OBJECT）
	void set_kinematic( bool is_kinematic );
	bool is_kinematic() const;

	/// 当たりの応答を消す（すり抜ける。当たりの判定は残る。CF_NO_CONTACT_RESPONSE）
	void set_no_contact_response( bool is_no_contact_response );
	bool has_contact_response() const;

	/// 連続衝突判定（薄い壁を速い物が貫通しないようにする）
	void set_ccd( float_t swept_sphere_radius, float_t motion_threshold );
	float_t get_ccd_swept_sphere_radius() const;
	float_t get_ccd_motion_threshold() const;

	/// 形の種類
	RigidBodyShapeType get_shape_type() const;

	// ---- Entity の番号のタグ（core/physics/EntityTag.h。当たり・レイの結果で Entity を引く） ----

	void set_entity_tag( ecs::EntityId id );
	void clear_entity_tag();
	std::optional< ecs::EntityId > get_entity_tag() const;

}; // class RigidBody

} // namespace core::physics
