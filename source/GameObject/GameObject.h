#pragma once

#include <blue_sky/type.h>
#include <functional>

class btRigidBody;

namespace game
{

class Sound;

} // namespace game

namespace blue_sky
{

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
 * ゲーム上に存在するすべてのオブジェクトはトランスフォーム ( 移動・回転・拡大縮小 ) 情報を持つ
 * ゲーム上に存在するすべてのオブジェクトはオプションとして剛体 ( 衝突判定用の形状を含む ) 情報を持つ
 *
 * @todo Bullet Physics を隠蔽する
 */
class GameObject
{
public:
	using Sound			= game::Sound;
	using EventHandler	= std::function< void() >;

	static Vector GravityDefault;

private:
	Transform		transform_;		///< トランスフォーム情報
	btRigidBody*	rigid_body_;	///< 剛体情報

protected:
	float_t get_frame_elapsed_time() const;
	float_t get_scene_elapsed_time() const;

	/// 1 秒あたりの数値から現在のフレームの数値を返す
	inline float_t per_sec( float_t value ) const { return value * get_frame_elapsed_time(); }

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

	/// @todo 分離する
	void set_mass( float_t );
	void set_gravity( const Vector& );
	void set_friction( float_t );
	void set_angular_factor( float_t );
	void set_angular_factor( const Vector& );
	void set_kinematic( bool is_kinematic );
	void set_no_contact_response( bool is_no_contact_response );

	Transform& get_transform();
	const Transform& get_transform() const;
	
	/// @todo Bullet Physics を隠蔽する
	inline btRigidBody* get_rigid_body() { return rigid_body_; }
	inline const btRigidBody* get_rigid_body() const { return rigid_body_; }
	inline void set_rigid_body( btRigidBody* rigid_body ) { rigid_body_ = rigid_body; }

	const Vector& get_location() const { return get_transform().get_position(); }
	void set_location( const Vector& );
	void set_location( float_t, float_t, float_t );

	const Vector& get_velocity() const;
	void set_velocity( const Vector& );

	bool is_moving_to( const GameObject* ) const;

	virtual void add_event_handler( const char_t*, const EventHandler& ) { }

}; // class GameObject

} // namespace blue_sky
