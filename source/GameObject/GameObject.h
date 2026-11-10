#pragma once

#include <blue_sky/type.h>
#include <core/math/Transform.h>

#include <functional>

class btRigidBody;

namespace blue_sky { struct GameObjectContext; }

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
class Umbrella;
class Stone;
class BaseSwitch;
class Medal;
class Robot;
class StaticObject;
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
	Transform		transform_;		///< トランスフォーム情報
	btRigidBody*	rigid_body_;	///< 剛体情報
	const GameObjectContext* context_ = nullptr;	///< 物理の世界・音・経過時間

protected:
	float_t get_delta_time() const;
	float_t get_scene_elapsed_time() const;

	/// 物理のワールド（当たりの判定・レイの判定に使う。派生クラスは GameMain を直接呼ばずにこれを使う。設計上の問題 B28）
	/// 物理（ray_test・contact_test で当たりを調べる）
	const ActiveObjectPhysics* get_physics() const;

	/// 1 秒あたりの数値から現在のフレームの数値を返す
	inline float_t per_sec( float_t value ) const { return value * get_delta_time(); }

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
	GameObject( const GameObject& ) = default;

	/// 物理の世界・音・経過時間（ActiveObjectManager が加えるときに渡す。以前は GameMain のシングルトンから取っていた）
	void set_context( const GameObjectContext* context ) { context_ = context; }
	const GameObjectContext* get_context() const { return context_; }

	virtual ~GameObject();

	/// 
	virtual void restart() = 0;

	/// 更新
	virtual void update() = 0;
	virtual void update_transform();
	virtual void commit_transform();
	
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
	virtual void on_collide_with( Umbrella* ) { }
	virtual void on_collide_with( Stone* ) { }
	virtual void on_collide_with( BaseSwitch* ) { }
	virtual void on_collide_with( Robot* ) { }
	virtual void on_collide_with( Medal* ) { }
	virtual void on_collide_with( StaticObject* ) { }
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
	/// 剛体を設定する（ActiveObject は上書きして、剛体に自分の Entity の番号のタグを付ける）
	virtual void set_rigid_body( btRigidBody* rigid_body ) { rigid_body_ = rigid_body; }

	const Vector& get_location() const { return get_transform().get_position(); }
	void set_location( const Vector& );
	void set_location( float_t, float_t, float_t );

	const Vector& get_velocity() const;
	void set_velocity( const Vector& );

	bool is_moving_to( const GameObject* ) const;

	virtual void add_event_handler( const char_t*, const EventHandler& ) { }

}; // class GameObject

} // namespace blue_sky
