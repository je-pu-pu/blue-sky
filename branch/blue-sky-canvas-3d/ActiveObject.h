#ifndef BLUE_SKY_ACTIVE_OBJECT_H
#define BLUE_SKY_ACTIVE_OBJECT_H

class Direct3D11Mesh;

class btRigidBody;
class btTransform;

namespace blue_sky
{

class Input;
class Stage;
class GridCell;

/**
 * 行動するオブジェクト
 *
 */
class ActiveObject
{
public:
	typedef Direct3D11Mesh Mesh;
	typedef btTransform Transform;
	typedef btRigidBody RigidBody;

private:
	const Mesh*		mesh_;				///< Mesh
	RigidBody*		rigid_body_;		///< RigidBody
	Transform*		transform_;			///< Transform

protected:

public:
	ActiveObject();
	virtual ~ActiveObject();

	/// 更新
	virtual void update() = 0;
	void update_transform();
	
	virtual float get_collision_width() const = 0;
	virtual float get_collision_height() const = 0;
	virtual float get_collision_depth() const = 0;

	void set_mesh( const Mesh* m ) { mesh_ = m; }
	const Mesh* get_mesh() const { return mesh_; }

	void set_rigid_body( RigidBody* rigid_body ) { rigid_body_ = rigid_body; }

	Transform& get_transform();
	const Transform& get_transform() const;

}; // class ActiveObject

} // namespace blue_sky

#endif // BLUE_SKY_ACTIVE_OBJECT_H
