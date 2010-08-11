#ifndef BLUE_SKY_PLAYER_H
#define BLUE_SKY_PLAYER_H

#include "ActiveObject.h"

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

	enum ItemType
	{
		ITEM_TYPE_NONE = -1,
		ITEM_TYPE_ROCKET = 0,
		ITEM_TYPE_UMBRELLA,
		ITEM_TYPE_SCOPE,
		ITEM_TYPE_MAX
	};

private:
	const Input* input_;				///< ���͂ւ̎Q��

	int			step_count_;
	float		step_speed_;			///< �ړ����x
	float		gravity_;				///< �d��
	float		super_jump_velocity_;	///< �X�[�p�[�W�����v���i��

	float		eye_height_;			///< �ڂ̍���

	bool		is_turn_avaiable_;		///< �����]���L���t���O
	bool		is_jumping_;			///< �W�����v���t���O
	bool		is_clambering_;			///< �悶�o�蒆�t���O

	bool		is_falling_;			///< �������t���O
	vector3		velocity_on_fall_;		///< �����J�n���̈ړ���

	ActionMode	action_mode_;			///< ���݂̃A�N�V�����̃��[�h
	vector3		action_base_position_;	///< ���݂̃A�N�V�����̊��ʒu
	bool		is_action_pre_finish_;	///< ���݂̃A�N�V���������������I���t���O

	bool		has_medal_;				///< ���_���ێ��t���O

	int			item_count_[ ITEM_TYPE_MAX ];
	ItemType	selected_item_type_;	///< ���ݑI�𒆂̃A�C�e��

	bool		look_floor_request_;	///< �������ė~�����v��

	void on_collision_x( const GridCell& );
	void on_collision_y( const GridCell& );
	void on_collision_z( const GridCell& );

	float get_collision_width() const;
	float get_collision_height() const;
	float get_collision_depth() const;

	float get_clambering_speed() const;

	float get_balloon_action_length() const { return 10.f; }
	float get_rocket_action_length() const { return 60.f; }

public:
	Player();
	~Player() { }
	
	void add_direction_degree( float );

	float get_eye_height() const { return eye_height_; }

	void step( float );
	void side_step( float );

	void stop();

	void update_step_speed();

	void kill();

	void rebirth();

	bool is_turn_available() const { return is_turn_avaiable_; }

	float get_step_speed() const { return step_speed_; }
	float get_side_step_speed() const { return 0.005f; }

	/// �X�V
	void update();
	
	/// �W�����v����
	void jump();
	bool is_jumping() const { return is_jumping_; }

	bool is_clambering() const { return is_clambering_; }
	bool is_rocketing() const { return action_mode_ == ACTION_MODE_ROCKET; }

	/// ��������
	void fall();
	bool is_falling() const { return is_falling_; }
	bool is_falling_to_dead() const;
	
	bool is_if_fall_to_dead( float ) const;
	bool is_if_fall_to_umbrella_lost( float ) const;

	///
	void start_umbrella_mode();
	void lose_umbrella();

	void switch_scope_mode();

	void set_input( const Input* );

	float get_gravity() const { return gravity_; }
	void set_gravity( float g ) { gravity_ = g; }
	
	void rocket( const vector3& );
	void stop_rocket();

	void on_get_balloon();
	void on_get_rocket();
	void on_get_umbrella();
	void on_get_medal();

	bool has_medal() const { return has_medal_; }

	ActionMode get_action_mode() const { return action_mode_; }
	void set_action_mode( ActionMode );

	bool is_action_pre_finish() const { return is_action_pre_finish_; }


	ItemType get_selected_item_type() const { return selected_item_type_; }
	int get_item_count( ItemType ) const;

	void select_prev_item();
	void select_next_item();

	void push_look_floor_request() { look_floor_request_ = true; }
	bool pop_look_floor_request() { bool r = look_floor_request_; look_floor_request_ = false; return r; }

}; // class Player

} // namespace blue_sky

#endif // BLUE_SKY_PLAYER_H
