#pragma once

#include <type/type.h>

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

#include <functional>

class btVector3;
class btTransform;
class btRigidBody;

namespace game
{

class Sound;

} // namespace game

namespace blue_sky
{

class DrawingModel;

class Player;
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
 */
class GameObject
{
public:
	typedef game::Sound			Sound;

	typedef btScalar			Scalar;
	typedef btVector3			Vector3;
	typedef btMatrix3x3			Matrix;
	typedef btQuaternion		Quaternion;
	typedef btTransform			Transform;	
	typedef btRigidBody			RigidBody;
	typedef btDynamicsWorld		DynamicsWorld;

	typedef std::function< void() > EventHandler;

	static Vector3 GravityDefault;
	static Vector3 GravityZero;

private:
	Transform	transform_;			///< Transform
	RigidBody*	rigid_body_;		///< RigidBody

protected:
	float_t get_frame_elapsed_time() const;
	float_t get_scene_elapsed_time() const;

	/// 1 秒あたりの数値から現在のフレームの数値を返す
	inline float_t per_sec( float_t value ) const { return value * get_frame_elapsed_time(); }

	float_t get_flicker_height_offset( float_t scale = 1.f ) const;
	void update_location_by_flicker( const Vector3& base_location, float_t scale = 1.f );
	void update_velocity_by_flicker( const Vector3& base_location, float_t scale = 1.f );

	bool_t is_visible_in_blink( float_t ) const;

	DynamicsWorld* get_dynamics_world() const; ///< @todo 削除する

	void play_animation( const char_t* ) const;

	void play_sound( const char* name, bool loop = false, bool force = true ) const;
	void stop_sound( const char* name ) const;

	void fade_in_sound( const char* ) const;
	void fade_out_sound( const char* ) const;

public:
	GameObject();
	GameObject( const GameObject& ) = default;

	virtual ~GameObject();

	/// 
	virtual void restart() = 0;

	/// 更新
	virtual void update() = 0;
	virtual void update_transform();
	virtual void commit_transform();
	
	virtual void update_velocity_by_target_location( const Vector3& target_location, float_t speed );

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
	virtual const Vector3& get_default_gravity() const { return is_hard() ? GravityDefault : GravityZero; }

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

	void set_mass( float_t );
	void set_gravity( const Vector3& );
	void set_angular_factor( const Vector3& );
	void set_kinematic( bool is_kinematic );
	void set_no_contact_response( bool is_no_contact_response );

	inline RigidBody* get_rigid_body() { return rigid_body_; }
	inline const RigidBody* get_rigid_body() const { return rigid_body_; }

	inline void set_rigid_body( RigidBody* rigid_body ) { rigid_body_ = rigid_body; }

	Transform& get_transform();
	const Transform& get_transform() const;
	
	const Vector3& get_location() const { return get_transform().getOrigin(); }
	void set_location( const Vector3& );
	void set_location( float_t, float_t, float_t );

	const Vector3& get_velocity() const;
	void set_velocity( const Vector3& );

	bool is_moving_to( const GameObject* ) const;

	virtual void add_event_handler( const char_t*, const EventHandler& ) { }

}; // class GameObject

} // namespace blue_sky
