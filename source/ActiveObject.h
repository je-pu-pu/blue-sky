#ifndef BLUE_SKY_ACTIVE_OBJECT_H
#define BLUE_SKY_ACTIVE_OBJECT_H

#include "GameObject.h"
#include "ConstantBuffer.h"

class AnimationPlayer;

namespace blue_sky
{

/**
 * �s������I�u�W�F�N�g
 *
 */
class ActiveObject : public GameObject
{
public:

private:
	const DrawingModel*						drawing_model_;				///< DrawingModel
	const ObjectConstantBuffer*				object_constant_buffer_;	///< �萔�o�b�t�@
	AnimationPlayer*						animation_player_;			///< �A�j���[�V�����Đ�

	bool				is_dead_;			///< ���S�t���O

	/** @todo ActiveObject ���番�� */
	float_t				direction_degree_;	///< ���� ( Y Axis Degree )

	Vector3				start_location_;	///< �X�^�[�g���̈ʒu
	Vector3				start_rotation_;	///< �X�^�[�g���̉�] ( Degree )
	float_t				start_direction_degree_;	///< �X�^�[�g���̌��� ( Y Axis )

	Vector3				front_;				///< �O
	Vector3				right_;				///< �E


protected:
	virtual void limit_velocity();

	float get_max_speed() const { return 20.f; }
	
	void play_animation( const char_t* ) const;

public:
	ActiveObject();
	virtual ~ActiveObject();

	virtual void restart();

	virtual void update();

	virtual void set_drawing_model( const DrawingModel* m ) { drawing_model_ = m; }
	virtual const DrawingModel* get_drawing_model() const { return drawing_model_; }

	const ObjectConstantBuffer* get_object_constant_buffer() const { return object_constant_buffer_; }

	void setup_animation_player();
	
	AnimationPlayer* get_animation_player() { return animation_player_; }
	const AnimationPlayer* get_animation_player() const { return animation_player_; }

	void set_start_location( float_t, float_t, float_t );
	void set_start_rotation( float_t, float_t, float_t );
	void set_start_direction_degree( float_t );

	float_t get_direction_degree() const { return direction_degree_; }
	void set_direction_degree( float_t );
	void chase_direction_degree( float_t, float_t );

	Vector3& get_front() { return front_; }
	const Vector3& get_front() const { return front_; }

	Vector3& get_right() { return right_; }
	const Vector3& get_right() const { return right_; }

	Vector3& get_start_location() { return start_location_; }
	const Vector3& get_start_location() const { return start_location_; }

	virtual void kill();
	
	bool is_dead() const { return is_dead_; }

	/// �`�悳��邩�ǂ�����Ԃ�
	virtual bool is_visible() const { return ! is_dead(); }

}; // class ActiveObject

} // namespace blue_sky

#endif // BLUE_SKY_ACTIVE_OBJECT_H
