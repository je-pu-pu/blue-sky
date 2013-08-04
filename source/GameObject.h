#ifndef BLUE_SKY_GAME_OBJECT_H
#define BLUE_SKY_GAME_OBJECT_H

#include "type.h"
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

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
class Switch;
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

private:
	RigidBody*			rigid_body_;		///< RigidBody
	Transform*			transform_;			///< Transform

protected:
	float_t get_frame_elapsed_time() const;
	float_t get_scene_elapsed_time() const;

	bool_t is_visible_in_blink( float_t ) const;

	DynamicsWorld* get_dynamics_world() const;

	void play_animation( const char_t* ) const;

	void play_sound( const char*, bool = false, bool = true ) const;
	void stop_sound( const char* ) const;

	void fade_in_sound( const char* ) const;
	void fade_out_sound( const char* ) const;

public:
	GameObject();
	virtual ~GameObject();

	/// 
	virtual void restart() = 0;

	/// 更新
	virtual void update() = 0;
	virtual void update_transform();
	
	virtual bool is_hard() const { return true; }
	virtual bool is_block() const { return is_hard(); }
	virtual bool is_balloon() const { return false; }
	virtual bool is_dynamic_object() const { return false; }

	virtual float_t get_collision_width() const = 0;
	virtual float_t get_collision_height() const = 0;
	virtual float_t get_collision_depth() const = 0;
	
	virtual float_t get_height_offset() const { return get_collision_height() * 0.5f; }

	virtual void on_collide_with( GameObject* );

	virtual void on_collide_with( Player* ) { }
	virtual void on_collide_with( Balloon* ) { }
	virtual void on_collide_with( Rocket* ) { }
	virtual void on_collide_with( Umbrella* ) { }
	virtual void on_collide_with( Stone* ) { }
	virtual void on_collide_with( Switch* ) { }
	virtual void on_collide_with( Robot* ) { }
	virtual void on_collide_with( Medal* ) { }
	virtual void on_collide_with( StaticObject* ) { }
	virtual void on_collide_with( DynamicObject* ) { }
	virtual void on_collide_with( Ladder* ) { }

	virtual void on_collide_with_ground() { }

	void set_mass( float_t );

	inline RigidBody* get_rigid_body() { return rigid_body_; }
	inline const RigidBody* get_rigid_body() const { return rigid_body_; }

	inline void set_rigid_body( RigidBody* rigid_body ) { rigid_body_ = rigid_body; }

	Transform& get_transform();
	const Transform& get_transform() const;
	
	const Vector3& get_location() const { return transform_->getOrigin(); }
	void set_location( const Vector3& );
	void set_location( float_t, float_t, float_t );

	const Vector3& get_velocity() const;
	void set_velocity( const Vector3& );

	bool is_moving_to( const GameObject* ) const;

}; // class GameObject

} // namespace blue_sky

#endif // BLUE_SKY_GAME_OBJECT_H
