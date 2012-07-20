#ifndef BLUE_SKY_ACTIVE_OBJECT_H
#define BLUE_SKY_ACTIVE_OBJECT_H

#include "type.h"
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

class btVector3;
class btTransform;
class btRigidBody;

namespace blue_sky
{

class Input;
class Stage;
class GridCell;

class DrawingModel;

/**
 * 行動するオブジェクト
 *
 */
class ActiveObject
{
public:
	typedef btVector3	Vector3;
	typedef btMatrix3x3	Matrix;
	typedef btTransform Transform;
	typedef btRigidBody RigidBody;

private:
	const DrawingModel*	drawing_model_;		///< DrawingModel

	RigidBody*			rigid_body_;		///< RigidBody
	Transform*			transform_;			///< Transform

	/** @todo ActiveObject から分離 */
	Vector3				velocity_;			///< 移動量
	float_t				direction_degree_;	///< 方向 ( Y Axis Degree )

	Vector3				front_;				///< 前
	Vector3				right_;				///< 右

protected:

public:
	ActiveObject();
	virtual ~ActiveObject();

	/// 更新
	virtual void update() = 0;

	void update_rigid_body_velocity();
	void update_transform();
	
	virtual float get_collision_width() const = 0;
	virtual float get_collision_height() const = 0;
	virtual float get_collision_depth() const = 0;

	void set_drawing_model( const DrawingModel* m ) { drawing_model_ = m; }
	const DrawingModel* get_drawing_model() const { return drawing_model_; }

	inline RigidBody* get_rigid_body() { return rigid_body_; }
	inline const RigidBody* get_rigid_body() const { return rigid_body_; }

	inline void set_rigid_body( RigidBody* rigid_body ) { rigid_body_ = rigid_body; }

	void step( float_t );
	void side_step( float_t );

	void set_location( float_t, float_t, float_t );

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

}; // class ActiveObject

} // namespace blue_sky

#endif // BLUE_SKY_ACTIVE_OBJECT_H
