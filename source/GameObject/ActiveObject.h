#pragma once

#include "GameObject.h"
#include "DrawingModel.h"
#include <blue_sky/ShaderResources.h>

class AnimationPlayer;

namespace blue_sky
{

namespace graphics
{
	class Model;
}

/**
 * �s������I�u�W�F�N�g
 *
 */
class alignas( 16 ) ActiveObject : public GameObject
{
public:
	typedef graphics::Model Model;

private:
	Vector3				front_;				///< �O
	Vector3				right_;				///< �E

	Vector3				start_location_;	///< �X�^�[�g���̈ʒu
	Vector3				start_rotation_;	///< �X�^�[�g���̉�] ( Degree )
	float_t				start_direction_degree_;	///< �X�^�[�g���̌��� ( Y Axis )

	const DrawingModel*						drawing_model_;				///< DrawingModel
	const ObjectConstantBuffer*				object_constant_buffer_;	///< �萔�o�b�t�@ @todo �C���X�^���X���ɕK�v���H
	AnimationPlayer*						animation_player_;			///< �A�j���[�V�����Đ�

	bool				is_dead_;			///< ���S�t���O
	float_t				flicker_scale_;		///< ��炬�̑傫��

	/** @todo ActiveObject ���番�� */
	float_t				direction_degree_;	///< ���� ( Y Axis Degree )

	bool				is_mesh_visible_;	///< ���b�V����`�悷�邩�ǂ���
	bool				is_line_visible_;	///< ���C����`�悷�邩

protected:
	void setup_animation_player();

	virtual void limit_velocity();

	float get_max_speed() const { return 20.f; }

public:
	ActiveObject();
	ActiveObject( const ActiveObject& o );
	virtual ~ActiveObject();

	virtual ActiveObject* clone() const { COMMON_THROW_EXCEPTION_MESSAGE( "clone() not implemented." ); }

	virtual void restart();

	virtual void update() { }

	virtual void set_drawing_model( const DrawingModel* m ) { drawing_model_ = m; setup_animation_player(); }
	const DrawingModel* get_drawing_model() const { return drawing_model_; }

	void set_model( Model* m ) { drawing_model_ = static_cast< DrawingModel* >( m ); }
	Model* get_model() { return const_cast< DrawingModel* >( drawing_model_ ); }
	// const Model* get_model() const { return drawing_model_; }

	virtual void action( const string_t& );

	virtual void set_flicker_scale( float_t s ) { flicker_scale_ = s; }
	virtual float_t get_flicker_scale() const { return flicker_scale_; }

	AnimationPlayer* get_animation_player() { return animation_player_; }
	const AnimationPlayer* get_animation_player() const { return animation_player_; }

	void set_start_location( float_t, float_t, float_t );
	void set_start_rotation( float_t, float_t, float_t );
	void set_start_direction_degree( float_t );

	float_t get_direction_degree() const { return direction_degree_; }
	void set_direction_degree( float_t );
	void chase_direction_degree( float_t, float_t );
	void chase_direction_to( const Vector3&, float_t );

	Vector3& get_front() { return front_; }
	const Vector3& get_front() const { return front_; }

	Vector3& get_right() { return right_; }
	const Vector3& get_right() const { return right_; }

	Vector3& get_start_location() { return start_location_; }
	const Vector3& get_start_location() const { return start_location_; }

	virtual void kill();
	
	bool is_dead() const { return is_dead_; }

	/// �`�悳��邩�ǂ�����Ԃ�
	virtual bool is_visible() const { return ( is_mesh_visible() || is_line_visible() ); }

	virtual bool is_mesh_visible() const { return is_mesh_visible_; }
	virtual bool is_line_visible() const { return is_line_visible_; }

	void set_mesh_visible( bool v ) { is_mesh_visible_ = v; }
	void set_line_visible( bool v ) { is_line_visible_ = v; }

	const ObjectConstantBuffer* get_object_constant_buffer() const { return object_constant_buffer_; }

	void update_render_data() const;
	virtual void bind_render_data() const;

	virtual void render_mesh() const;
	virtual void render_line() const;

	void play_animation( const char_t* name, bool force, bool loop );

	void action( const string_t& s ) const;

	// virtual void render_line() = 0;

	static void* operator new ( size_t size )
	{
        return _aligned_malloc( size, 16 );
    }

	static void operator delete ( void* p )
	{
		_aligned_free( p );
	}

}; // class ActiveObject

#define DEFINE_ACTIVE_OBJECT_COMMON_FUNCTIONS \
	ActiveObject* clone() const override { return new std::remove_const< std::remove_reference< decltype( *this ) >::type >::type( * this ); }

} // namespace blue_sky
