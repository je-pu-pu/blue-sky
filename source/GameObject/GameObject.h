#pragma once

#include <blue_sky/type.h>
#include <core/math/Transform.h>

#include <functional>

class btRigidBody;

namespace blue_sky { struct GameObjectContext; }
namespace blue_sky::ecs { enum class PickupKind; }

namespace core
{

class Sound;

} // namespace core

namespace blue_sky
{

class Player;
class ActiveObjectPhysics;
class Balloon;
class Rocket;
class Stone;
class BaseSwitch;
class Robot;
class DynamicObject;
class Ladder;

/**
 * ゲーム上に存在する全てのオブジェクトの基底クラス
 *
 * ゲーム上に存在するすべてのオブジェクトはトランスフォーム ( 移動・回転・拡大縮小 ) 情報を持つ
 * ゲーム上に存在するすべてのオブジェクトはオプションとして剛体 ( 衝突判定用の形状を含む ) 情報を持つ
 *
 * Bullet Physics を直接参照している。ECS（core::ecs）への移し方は doc/ECS移行計画.md。
 * 以前あった ECS への橋渡し（ComponentMap・add_component・get_component）は使う所が無く、定義の無い
 * blue_sky::Component を指していたので消した
 */
class GameObject
{
public:
	using Sound			= core::Sound;
	using EventHandler	= std::function< void() >;

	static Vector GravityDefault;

private:
	Transform		own_transform_;					///< 自分で持つトランスフォーム（位置の正を部品へ移す前・移した後に戻したとき）
	Transform*		transform_ = & own_transform_;	///< 位置の正（自分の own_transform_ か、ActiveObject の Entity の TransformComponent）
	btRigidBody*	rigid_body_;	///< 剛体情報（持ち主ではない。本編の剛体の持ち主は ActiveObject の Entity の部品 ecs::PhysicsBodyComponent）
	const GameObjectContext* context_ = nullptr;	///< 物理の世界・音・経過時間

protected:
	/**
	 * 位置の正を transform にする（ActiveObject が、自分の Entity の TransformComponent を渡す）
	 *
	 * 今の位置を transform へ写してから切り替える。transform は unbind_transform を呼ぶまで生きていること
	 */
	void bind_transform( Transform* transform );

	/**
	 * 位置の正を自分で持つ形に戻す（Entity を消す前に呼ぶ）
	 *
	 * @param keep_current 今の位置を写してから戻すか（bind_transform で渡した先がもう無いときは false）
	 */
	void unbind_transform( bool keep_current = true );

	/// 剛体を指す（持ち主ではない）
	void set_rigid_body_view( btRigidBody* rigid_body ) { rigid_body_ = rigid_body; }

	float_t get_delta_time() const;
	float_t get_scene_elapsed_time() const;

	/// 物理のワールド（当たりの判定・レイの判定に使う。派生クラスは GameMain を直接呼ばずにこれを使う。設計上の問題 B28）
	/// 物理（ray_test・contact_test で当たりを調べる）
	const ActiveObjectPhysics* get_physics() const;

	/// 1 秒あたりの数値から現在のフレームの数値を返す
	inline float_t per_sec( float_t value ) const { return value * get_delta_time(); }

	/**
	 * ゆらぎの位相を決める種（物体ごとに決まった値。起動ごとに変わらないこと）
	 *
	 * ActiveObject は自分の Entity の番号を返す（ステージの物体は書いた順に作るので、番号は起動ごとに同じ）
	 */
	virtual unsigned int get_flicker_seed() const { return 0; }

	float_t get_flicker_height_offset( float_t scale = 1.f ) const;
	void update_location_by_flicker( const Vector& base_location, float_t scale = 1.f );
	void update_velocity_by_flicker( const Vector& base_location, float_t scale = 1.f );

	bool_t is_visible_in_blink( float_t ) const;

	void play_animation( const char_t* ) const;

	void play_sound( const char* name, bool loop = false, bool force = true ) const;
	void stop_sound( const char* name ) const;

	void fade_in_sound( const char* ) const;
	void fade_out_sound( const char* ) const;

public:
	GameObject();

	/// 位置は値で写す（位置の正は自分の own_transform_ に戻す。複製の Entity は加えるときに作る）。剛体は共有する（以前と同じ）
	GameObject( const GameObject& );
	GameObject& operator = ( const GameObject& ) = delete;

	/// 物理の世界・音・経過時間（ActiveObjectManager が加えるときに渡す。以前は GameMain のシングルトンから取っていた）
	void set_context( const GameObjectContext* context ) { context_ = context; }
	const GameObjectContext* get_context() const { return context_; }

	virtual ~GameObject();

	/// 
	virtual void restart() = 0;

	/// 更新
	virtual void update() = 0;

	/// 剛体の位置と回転を位置の正へ写す（本編の物体は ecs::PhysicsSyncSystem::update_transforms がまとめて行う）
	void update_transform();

	/// 位置の正を剛体へ送る（計算は ecs::PhysicsSyncSystem::write_body）
	void commit_transform();
	
	virtual void update_velocity_by_target_location( const Vector& target_location, float_t speed );

	/// 通り抜けられるオブジェクトかどうかを返す
	virtual bool is_ghost() const { return false; }

	/// ぶつかった時に反発のある、硬いオブジェクトかどうかを返す
	virtual bool is_hard() const { return true; }

	/// 壁や障害物として有効なオブジェクトどうかを返す
	virtual bool is_block() const { return is_hard(); }

	/// ( 風船やゴールなど ) そこに着地しても死なない足場かどうかを返す
	virtual bool is_safe_footing() const { return false; }

	virtual float_t get_collision_width() const = 0;
	virtual float_t get_collision_height() const = 0;
	virtual float_t get_collision_depth() const = 0;
	
	virtual float_t get_height_offset() const { return get_collision_height() * 0.5f; }

	virtual float_t get_default_mass() const { return 1.f; }
	virtual const Vector& get_default_gravity() const { return is_hard() ? GravityDefault : Vector::Zero; }

	virtual void on_arrive_at_target_location() { }

	virtual void on_collide_with( GameObject* );

	virtual void on_collide_with( Player* ) { }
	virtual void on_collide_with( Balloon* ) { }
	virtual void on_collide_with( Rocket* ) { }
	virtual void on_collide_with( Stone* ) { }
	virtual void on_collide_with( BaseSwitch* ) { }
	virtual void on_collide_with( Robot* ) { }
	/**
	 * 拾う物（ECS だけの物体。ecs::PickupComponent を持つ）に当たった
	 *
	 * 拾ったら true を返す（当たりの互換の層が、拾われた物を死なせる）。拾わない物体は false（既定）。
	 * 以前の on_collide_with( Medal* )・on_collide_with( Rocket* )（Medal・Rocket・Umbrella クラスは消していく）
	 */
	virtual bool on_pick_up( ecs::PickupKind ) { return false; }

	/// 動かない物（ECS だけの物体。StaticObjectComponent を持つ）に当たった。以前の on_collide_with( StaticObject* )（StaticObject クラスは消した）
	virtual void on_collide_with_static_object() { }
	virtual void on_collide_with( DynamicObject* ) { }
	virtual void on_collide_with( Ladder* ) { }

	virtual void on_collide_with_ground() { }

	// Bullet Physics 操作
	void set_mass( float_t );
	void set_gravity( const Vector& );
	void set_friction( float_t );
	void set_angular_factor( float_t );
	void set_angular_factor( const Vector& );
	void set_kinematic( bool is_kinematic );
	void set_no_contact_response( bool is_no_contact_response );

	Transform& get_transform();
	const Transform& get_transform() const;
	
	inline btRigidBody* get_rigid_body() { return rigid_body_; }
	inline const btRigidBody* get_rigid_body() const { return rigid_body_; }

	const Vector& get_location() const { return get_transform().get_position(); }
	void set_location( const Vector& );
	void set_location( float_t, float_t, float_t );

	const Vector& get_velocity() const;
	void set_velocity( const Vector& );

	bool is_moving_to( const GameObject* ) const;

	virtual void add_event_handler( const char_t*, const EventHandler& ) { }

}; // class GameObject

} // namespace blue_sky
