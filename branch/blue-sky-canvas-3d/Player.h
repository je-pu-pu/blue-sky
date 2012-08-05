#ifndef BLUE_SKY_PLAYER_H
#define BLUE_SKY_PLAYER_H

#include "ActiveObject.h"

#include <common/math.h>

namespace blue_sky
{

/**
 * �v���C���[
 *
 */
class Player : public ActiveObject
{
public:
	enum ActionMode
	{
		ACTION_MODE_NONE = 0,
		ACTION_MODE_BALLOON,
		ACTION_MODE_ROCKET,
		ACTION_MODE_UMBRELLA,
		ACTION_MODE_SCOPE,
	};

private:
	bool		is_on_footing_;			///< ���ݑ����t���O
	bool		is_jumping_;			///< ���݃W�����v���t���O
	bool		is_jumpable_;			///< ���݃W�����v�\�t���O
	bool		is_falling_to_die_;		///< ���ݎ��S�֌����ė������t���O
	
	int			step_count_;			///< �ړ��J�E���^
	float		step_speed_;			///< �ړ����x

	ActionMode	action_mode_;			///< ���݂̃A�N�V�����̃��[�h
	Vector3		action_base_position_;	///< ���݂̃A�N�V�����̊��ʒu
	bool		is_action_pre_finish_;	///< ���݂̃A�N�V���������������I���t���O

	float_t		uncontrollable_timer_;	///< ����s�\�^�C�}�[

	float_t		eye_height_;			///< �ڂ̍���
	float_t		eye_depth_;				///< �ڂ̉��s���I�t�Z�b�g ( �g�����o�������ɑ����� ) ( 0.f .. 1.f )

	bool		has_medal_;				///< ���_���ێ��t���O

	float		last_footing_height_;	///< �O��̑���̍��� ( �o���[���܂� )

	Balloon*	balloon_;				///< ���ݎ����Ă��镗�D

protected:
	const Input* get_input() const;
	
	bool is_last_footing_height_null() const { return last_footing_height_ >= 300.f; }
	
	void set_last_footing_height_to_null() { last_footing_height_ = 300.f; }
	void set_last_footing_height_to_current_height() { last_footing_height_ = get_location().y(); }

	void set_action_base_position_to_current_position() { action_base_position_ = get_location(); }

	float get_height_to_die() const { return 15.f; }
	float get_balloon_action_length() const { return 10.f; }

	float_t get_min_walk_step_speed() const { return 0.1f; }
	float_t get_max_walk_step_speed() const { return 0.25f; }
	float_t get_max_run_step_speed() const { return 0.5f; }

	float_t get_side_step_speed() const { return 0.25f; }

	bool is_walking() const { return get_step_speed() > get_min_walk_step_speed(); }
	bool is_running() const { return get_step_speed() > get_max_walk_step_speed(); }

	void update_on_footing();
	void update_jumpable();
	void update_jumping();
	
	void update_falling_to_die();

	void update_step_speed();

	void limit_velocity();

	bool check_on_footing( const Vector3&, float_t ) const;

	bool is_on_footing() const { return is_on_footing_; }
	bool is_jumpable() const { return is_jumpable_; }
	bool is_jumping() const { return is_jumping_; }

	bool is_uncontrollable() const { return uncontrollable_timer_ > 0.f; }

	void on_collide_with( ActiveObject* o ) { o->on_collide_with( this ); }
	void on_collide_with( Balloon* );
	void on_collide_with( Robot* );

public:
	Player();
	~Player() { }

	float_t get_collision_width() const { return 0.5f; }
	float_t get_collision_height() const { return 1.75f; }
	float_t get_collision_depth() const { return 0.25f; }

	float_t get_step_speed() const { return step_speed_; }

	float_t get_last_footing_height() const { return last_footing_height_; }

	void restart();

	/// �X�V
	void update();
	
	// �ړ�
	void step( float_t );
	void side_step( float_t );

	/// �W�����v����
	void jump();

	// ��~
	void stop();

	/// �������Z
	void add_direction_degree( float );

	void damage( const Vector3& );
	void kill();

	void set_eye_depth( float );
	void add_eye_depth( float );

	float_t get_eye_height() const { return eye_height_; }
	float_t get_eye_depth() const { return eye_depth_; }

	//
	bool is_falling() const;
	bool is_falling_to_die() const { return is_falling_to_die_; }

	bool has_medal() const { return has_medal_; }

	ActionMode get_action_mode() const { return action_mode_; }
	void set_action_mode( ActionMode );

	const Balloon* get_balloon() const { return balloon_; }


}; // class Player

} // namespace blue_sky

#endif // BLUE_SKY_PLAYER_H
