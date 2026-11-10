#pragma once

#include <blue_sky/ecs/component/TargetMoverComponent.h>
#include <core/ecs/type.h>
#include <core/type.h>

class btRigidBody;

namespace core::sound { class SoundManager; }
namespace core { class AnimationPlayer; }

namespace core::ecs
{
	class Entity;
	class TransformComponent;
}

namespace blue_sky::ecs
{

struct PhysicsBodyComponent;
struct SimpleObjectComponent;
struct CollisionTraitsComponent;
struct ObjectRenderComponent;

/**
 * ECS だけの物体（ActiveObject を持たない物体）を、部品を通して動かす窓口
 *
 * 以前 ActiveObject・GameObject のメンバ関数がしていたこと（位置・速度・向き・やり直し・死亡・表示）を、
 * 同じ決まりで部品（TransformComponent・PhysicsBodyComponent・SimpleObjectComponent・CollisionTraitsComponent・ObjectRenderComponent）に対して行う。
 * 位置の正は TransformComponent、剛体は PhysicsBodyComponent（位置を変えたらその場で剛体へ送る。以前の commit_transform と同じ）。
 *
 * 持ち続けない（使うときにその場で作る）。部品の場所は外すまで動かないので、作ってから使い終わるまでの間に Entity を消さなければよい。
 * 目標地点・向きへ動かす（MoveToTargetSystem）ときの窓口（TargetMover）にもなる（ActiveObject を持たない物体の TargetMoverComponent::mover は nullptr）
 */
class EntityObject : public TargetMover
{
private:
	core::ecs::Entity*				entity_;
	core::ecs::TransformComponent*	transform_;		///< 位置の正（無ければ何も動かさない）
	PhysicsBodyComponent*			body_;			///< 剛体（無ければ nullptr。剛体の無い物体は速度を持たない）
	SimpleObjectComponent*			state_;			///< やり直しの状態と向き（無ければ nullptr）

	void commit_transform();

public:
	explicit EntityObject( core::ecs::Entity* entity );

	/// 位置の正（TransformComponent）があるか
	bool is_valid() const { return transform_ != nullptr; }

	core::ecs::Entity* get_entity() const { return entity_; }
	btRigidBody* get_rigid_body() const;

	const core::Vector& get_location() const;
	void set_location( const core::Vector& location );

	/// 位置と回転（位置の正。足元の位置）
	const core::Transform& get_transform() const;

	/// 速度（剛体が無ければ 0）
	core::Vector get_velocity() const;
	/// 速度を設定する（剛体が無ければ何もしない）
	void set_velocity( const core::Vector& velocity );

	/// 指定した位置に向かう速度を設定する（以前の GameObject::update_velocity_by_target_location）
	void update_velocity_by_target_location( const core::Vector& target_location, float_t speed );

	float_t get_direction_degree() const;
	void set_direction_degree( float_t degree );
	void chase_direction_degree( float_t degree, float_t speed );
	void chase_direction_to( const core::Vector& location, float_t speed );

	/// 前（向きから決める。以前の ActiveObject::get_front と同じ計算）
	core::Vector get_front() const;
	/// 右（同上。以前の ActiveObject::get_right）
	core::Vector get_right() const;

	/// やり直したときの位置（SimpleObjectComponent が無ければ今の位置）
	core::Vector get_start_location() const;

	/// ゆらぎの位相を決める種（自分の Entity の番号。以前の ActiveObject::get_flicker_seed と同じ）
	unsigned int get_flicker_seed() const;

	/// 指定した位置にゆらぎの高さのずれを足して置く（以前の GameObject::update_location_by_flicker）
	void update_location_by_flicker( const core::Vector& base_location, float_t scale, float_t scene_elapsed_time );
	/// 指定した位置にゆらぎの高さのずれを足した所へ、上下の速度を向ける（以前の GameObject::update_velocity_by_flicker）
	void update_velocity_by_flicker( const core::Vector& base_location, float_t scale, float_t scene_elapsed_time );

	void set_start_location( const core::Vector& location );
	void set_start_rotation( float_t x, float_t y, float_t z );
	void set_start_direction_degree( float_t degree );

	/// やり直す（以前の ActiveObject::restart と同じ : 生き返らせ、剛体があれば始めの位置・回転に戻して止め、向きを戻し、表示する）
	void restart();

	/// 死なせる（当たりを起こさず、描かない。以前の ActiveObject::kill。風船は (0, -100, 0) へ退く。以前の Balloon::kill）
	void kill();
	bool is_dead() const;

	/// メッシュ・線を描くか（以前の ActiveObject::set_mesh_visible・set_line_visible）
	void set_visible( bool mesh, bool line );

	/// アニメーションを再生する（アニメーションの無い物体は何もしない。以前の ActiveObject::play_animation）
	void play_animation( const char_t* name, bool force, bool loop );

	/// アニメーションの再生（無ければ nullptr）
	core::AnimationPlayer* get_animation_player() const;

	/**
	 * 物体の位置で音を鳴らす（以前の GameObject::play_sound と同じ。既定は鳴っていても初めから鳴らし直し、繰り返さない）
	 *
	 * @param sound_manager 音を引く先（nullptr・音が無ければ何もしない）
	 * @param loop 繰り返すか
	 * @param force 鳴っていても鳴らし直すか（false なら、鳴っている間は何もしない）
	 */
	void play_sound( core::sound::SoundManager* sound_manager, const char_t* name, bool loop = false, bool force = true ) const;

	/// 音を止める（以前の GameObject::stop_sound。音は物体ごとではなく名前ごとに 1 つ）
	static void stop_sound( core::sound::SoundManager* sound_manager, const char_t* name );

	// TargetMover（MoveToTargetSystem が呼ぶ）
	core::Vector get_target_mover_location() const override { return get_location(); }
	void move_toward_target_location( const core::Vector& target_location, float_t speed ) override { update_velocity_by_target_location( target_location, speed ); }
	void stop_at_target_location( const core::Vector& target_location ) override;
	void notify_arrive_at_target_location() override;		///< 少女は止まり、そこが浮く中心になる（以前の Girl::on_arrive_at_target_location）。ほかの種類はすることが無い
	void chase_target_direction_degree( float_t degree, float_t speed ) override { chase_direction_degree( degree, speed ); }
	void chase_target_direction_to( const core::Vector& location, float_t speed ) override { chase_direction_to( location, speed ); }
};

} // namespace blue_sky::ecs
