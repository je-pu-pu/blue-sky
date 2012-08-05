#ifndef BLUE_SKY_PLAYER_H
#define BLUE_SKY_PLAYER_H

#include "ActiveObject.h"

#include <common/math.h>

namespace blue_sky
{

/**
 * プレイヤー
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
	bool		is_on_footing_;			///< 現在足場上フラグ
	bool		is_jumping_;			///< 現在ジャンプ中フラグ
	bool		is_jumpable_;			///< 現在ジャンプ可能フラグ
	bool		is_falling_to_die_;		///< 現在死亡へ向けて落下中フラグ
	
	int			step_count_;			///< 移動カウンタ
	float		step_speed_;			///< 移動速度

	ActionMode	action_mode_;			///< 現在のアクションのモード
	Vector3		action_base_position_;	///< 現在のアクションの基底位置
	bool		is_action_pre_finish_;	///< 現在のアクションがもうすぐ終わるフラグ

	float_t		uncontrollable_timer_;	///< 制御不能タイマー

	float_t		eye_height_;			///< 目の高さ
	float_t		eye_depth_;				///< 目の奥行きオフセット ( 身を乗り出した時に増える ) ( 0.f .. 1.f )

	bool		has_medal_;				///< メダル保持フラグ

	float		last_footing_height_;	///< 前回の足場の高さ ( バルーン含む )

	Balloon*	balloon_;				///< 現在持っている風船

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

	/// 更新
	void update();
	
	// 移動
	void step( float_t );
	void side_step( float_t );

	/// ジャンプ処理
	void jump();

	// 停止
	void stop();

	/// 方向加算
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
