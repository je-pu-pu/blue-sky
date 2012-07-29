#ifndef BLUE_SKY_ACTIVE_OBJECT_H
#define BLUE_SKY_ACTIVE_OBJECT_H

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

class Input;
class Stage;
class GridCell;

class DrawingModel;

/**
 * �s������I�u�W�F�N�g
 *
 */
class ActiveObject
{
public:
	typedef game::Sound			Sound;

	typedef btVector3			Vector3;
	typedef btMatrix3x3			Matrix;
	typedef btQuaternion		Quaternion;
	typedef btTransform			Transform;	
	typedef btRigidBody			RigidBody;
	typedef btDynamicsWorld		DynamicsWorld;

private:
	const DrawingModel*	drawing_model_;		///< DrawingModel

	bool				is_dead_;			///< ���S�t���O

	RigidBody*			rigid_body_;		///< RigidBody
	Transform*			transform_;			///< Transform

	/** @todo ActiveObject ���番�� */
	Vector3				velocity_;			///< �ړ���
	float_t				direction_degree_;	///< ���� ( Y Axis Degree )

	Vector3				start_location_;	///< �X�^�[�g���̈ʒu

	Vector3				front_;				///< �O
	Vector3				right_;				///< �E


protected:
	void limit_velocity();

	float get_max_speed() const { return 4.f; }
	
	float get_elapsed_time() const;

	DynamicsWorld* get_dynamics_world() const;


	void play_sound( const char*, bool = false, bool = true ) const;
	void stop_sound( const char* ) const;

public:

	ActiveObject();
	virtual ~ActiveObject();

	/// 
	virtual void restart() { }

	/// �X�V
	virtual void update() = 0;

	void update_rigid_body_velocity();
	virtual void update_transform();
	
	virtual float get_collision_width() const = 0;
	virtual float get_collision_height() const = 0;
	virtual float get_collision_depth() const = 0;

	void set_drawing_model( const DrawingModel* m ) { drawing_model_ = m; }
	const DrawingModel* get_drawing_model() const { return drawing_model_; }

	inline RigidBody* get_rigid_body() { return rigid_body_; }
	inline const RigidBody* get_rigid_body() const { return rigid_body_; }

	inline void set_rigid_body( RigidBody* rigid_body ) { rigid_body_ = rigid_body; }

	void set_start_location( float_t, float_t, float_t );

	void set_location( float_t, float_t, float_t );
	void set_rotation( float_t, float_t, float_t );

	Transform& get_transform();
	const Transform& get_transform() const;


	Vector3& get_velocity() { return velocity_; }
	const Vector3& get_velocity() const { return velocity_; }

	float get_direction_degree() const { return direction_degree_; }
	void set_direction_degree( float d );

	Vector3& get_front() { return front_; }
	const Vector3& get_front() const { return front_; }

	Vector3& get_right() { return right_; }
	const Vector3& get_right() const { return right_; }

	Vector3& get_start_location() { return start_location_; }
	const Vector3& get_start_location() const { return start_location_; }

	const Vector3& get_location() const { return transform_->getOrigin(); }

	virtual void kill();

	/*
	void render_shadow();

	const GridCell& get_floor_cell_center() const;
	const GridCell& get_floor_cell_left_front() const;
	const GridCell& get_floor_cell_right_front() const;
	const GridCell& get_floor_cell_left_back() const;
	const GridCell& get_floor_cell_right_back() const;
	*/
	
	bool is_dead() const { return is_dead_; }

}; // class ActiveObject

} // namespace blue_sky

#endif // BLUE_SKY_ACTIVE_OBJECT_H
