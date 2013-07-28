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

	enum ItemType
	{
		ITEM_TYPE_NONE = -1,
		ITEM_TYPE_ROCKET = 0,
		ITEM_TYPE_UMBRELLA,
		ITEM_TYPE_SCOPE,
		ITEM_TYPE_MAX
	};

private:
	bool		is_on_footing_;			///< 現在足場上フラグ
	bool		is_jumping_;			///< 現在ジャンプ中フラグ
	bool		is_jumpable_;			///< 現在ジャンプ可能フラグ
	bool		is_falling_to_die_;		///< 現在死亡へ向けて落下中フラグ
	bool		is_falling_to_balloon_;	///< そのまま落下すると風船を取得するフラグ
	
	bool		is_on_ladder_;			///< 梯子と接触中フラグ

	int			step_count_;			///< 移動カウンタ
	float		step_speed_;			///< 移動速度

	ActionMode	action_mode_;			///< 現在のアクションのモード
	Vector3		action_base_position_;	///< 現在のアクションの基底位置
	bool		is_action_pre_finish_;	///< 現在のアクションがもうすぐ終わるフラグ
	int_t		balloon_sequence_count_;///< 現在の連続取得バルーンの数 ( 0 .. 7 )

	float_t		uncontrollable_timer_;	///< 制御不能タイマー

	float_t		pitch_;					///< 上下の向き ( 真下 : -1.f / 真上 : 1.f )

	float_t		eye_height_;			///< 目の高さ
	float_t		eye_depth_;				///< 目の奥行きオフセット ( 身を乗り出した時に増える ) ( 0.f .. 1.f )

	bool		has_medal_;				///< メダル保持フラグ

	int			item_count_[ ITEM_TYPE_MAX ];	///< 各アイテムの個数
	ItemType	selected_item_type_;	///< 現在選択中のアイテム

	float		last_footing_height_;	///< 前回の足場の高さ ( バルーン含む )

	Ladder*		ladder_;				///< 現在接触している梯子
	Balloon*	balloon_;				///< 現在持っている風船

	int			hp_;					///< HP

protected:
	void set_last_footing_height_to_current_height() { last_footing_height_ = get_location().y(); }

	void set_action_base_position_to_current_position() { action_base_position_ = get_location(); }

	float get_height_to_die() const { return 15.f; }
	float get_balloon_action_length() const { return 5.f; }

	float_t get_min_walk_step_speed() const { return 0.1f; }
	float_t get_max_walk_step_speed() const { return 0.25f; }
	float_t get_max_run_step_speed() const { return 0.5f; }

	float_t get_side_step_speed() const { return 0.25f; }
	float_t get_ladder_step_speed() const { return 2.f; }

	bool is_walking() const { return get_step_speed() > get_min_walk_step_speed(); }
	bool is_running() const { return get_step_speed() > get_max_walk_step_speed(); }

	void update_on_footing();
	void update_jumpable();
	void update_jumping();
	
	void update_falling_to_die();

	void update_step_speed();

	void update_gravity();

	void limit_velocity();

	bool check_on_footing( const Vector3&, float_t, bool = false ) const;
	float_t get_footing_height( const Vector3&, bool = false ) const;

	bool is_uncontrollable() const { return uncontrollable_timer_ > 0.f; }

	void on_collide_with( GameObject* o ) { o->on_collide_with( this ); }
	void on_collide_with( Balloon* );
	void on_collide_with( Medal* );
	void on_collide_with( Robot* );
	void on_collide_with( Ladder* );

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
	void update_transform();
	
	// 移動
	void step( float_t );
	void side_step( float_t );
	void ladder_step( float_t );

	void release_ladder();

	/// ジャンプ処理
	void jump();
	void super_jump();

	// 停止
	void stop();

	/// 方向加算
	void add_direction_degree( float );

	void set_pitch( float_t p ) { pitch_ = p; }
	bool is_ladder_step_only() const;

	void damage( const Vector3& );
	void kill();

	void set_eye_depth( float );
	void add_eye_depth( float );

	float_t get_eye_height() const { return eye_height_; }
	float_t get_eye_depth() const { return eye_depth_; }

	//
	bool is_falling() const;
	bool is_falling_to_die() const { return is_falling_to_die_; }
	bool is_falling_to_balloon() const { return is_falling_to_balloon_; }
	
	bool is_on_footing() const { return is_on_footing_; }
	bool is_on_ladder() const { return is_on_ladder_; }
	bool is_jumpable() const { return is_jumpable_; }
	bool is_jumping() const { return is_jumping_; }

	bool has_medal() const { return has_medal_; }

	ActionMode get_action_mode() const { return action_mode_; }
	void set_action_mode( ActionMode );

	bool is_action_pre_finish() const { return is_action_pre_finish_; }

	ItemType get_selected_item_type() const { return selected_item_type_; }
	int get_item_count( ItemType ) const;

	void select_prev_item();
	void select_next_item();

	void switch_scope_mode();


	const Balloon* get_balloon() const { return balloon_; }

	int get_hp() const { return hp_; }

}; // class Player

} // namespace blue_sky

#endif // BLUE_SKY_PLAYER_H
