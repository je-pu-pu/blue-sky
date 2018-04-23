#ifndef BLUE_SKY_PLAYER_H
#define BLUE_SKY_PLAYER_H

#include "ActiveObject.h"
#include <common/math.h>
#include <list>

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
		ITEM_TYPE_STONE,
		ITEM_TYPE_SCOPE,
		ITEM_TYPE_MAX
	};

	typedef std::list< Stone* > StoneList;

private:
	bool		is_on_footing_;			///< 現在足場上フラグ
	bool		is_jumping_;			///< 現在ジャンプ中フラグ
	bool		is_jumpable_;			///< 現在ジャンプ可能フラグ
	bool		is_clambering_;			///< 現在よじ登り中フラグ

	bool		is_located_on_die_;		///< 現在死亡へ向けて落下中フラグ
	bool		is_located_on_safe_;	///< そのまま落下すると風船またはゴールと接触するフラグ
	
	bool		is_on_ladder_;			///< 梯子と接触中フラグ
	bool		is_facing_to_block_;	///< 障害物の方を向いて接触中フラグ
	bool		can_clamber_;			///< よじ登り可能フラグ
	bool		can_peer_down_;			///< のぞき込み可能フラグ
	bool		can_throw_;				///< 石投げ可能フラグ

	bool		is_flickering_;			///< 現在揺らぎ中フラグ

	int			step_count_;			///< 移動カウンタ
	float		step_speed_;			///< 移動速度

	ActionMode	action_mode_;			///< 現在のアクションのモード
	float_t		action_timer_;			///< 現在のアクションが開始してからの経過時間
	Vector3		action_base_position_;	///< 現在のアクションの基底位置
	bool		is_action_pre_finish_;	///< 現在のアクションがもうすぐ終わるフラグ
	int_t		balloon_sequence_count_;///< 現在の連続取得バルーンの数 ( 0 .. 7 )
	int_t		balloon_sound_request_;	///< 

	float_t		uncontrollable_timer_;	///< 制御不能タイマー

	float_t		pitch_;					///< 上下の向き ( 真下 : -1.f / 真上 : 1.f )

	float_t		eye_height_;			///< 目の高さ
	float_t		eye_depth_;				///< 目の奥行きオフセット ( m ) ( 身を乗り出した時に増える ) ( 0.f .. 1.f )

	bool		has_medal_;				///< メダル保持フラグ

	int			item_count_[ ITEM_TYPE_MAX ];	///< 各アイテムの個数
	ItemType	selected_item_type_;	///< 現在選択中のアイテム

	float		last_footing_height_;	///< 前回の足場の高さ ( バルーン含む )

	Ladder*		ladder_;				///< 現在接触している梯子
	Balloon*	balloon_;				///< 現在持っている風船
	StoneList	stone_list_;			///< 現在持っている石のリスト

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

	float_t get_max_run_velocity() const { return 4.f; }

	float_t get_rocket_initial_velocity() const { return 10.f; }
	float_t get_rocket_action_length() const { return 10.f; }

	bool is_walking() const { return get_step_speed() > get_min_walk_step_speed(); }
	bool is_running() const { return get_step_speed() > get_max_walk_step_speed(); }

	void update_on_footing();
	void update_jumpable();
	void update_jumping();
	void update_facing_to_block();
	void update_can_clamber();
	void update_can_peer_down();
	void update_can_throw();

	void update_located_to_die();

	void update_step_speed();

	void update_gravity();

	void limit_velocity();

	bool check_on_footing( const Vector3&, float_t, bool = false ) const;
	float_t get_footing_height( const Vector3&, bool = false ) const;

	bool is_uncontrollable() const { return uncontrollable_timer_ > 0.f; }

	void on_collide_with( GameObject* o ) override { o->on_collide_with( this ); }
	void on_collide_with( Balloon* ) override;
	void on_collide_with( Rocket* ) override;
	void on_collide_with( Medal* ) override;
	void on_collide_with( Robot* ) override;
	void on_collide_with( Ladder* ) override;
	void on_collide_with( Stone* ) override;
	void on_collide_with( StaticObject* ) override;
	void on_collide_with( DynamicObject* ) override;

public:
	Player();
	~Player() { }

	float_t get_collision_width() const override { return 0.5f; }
	float_t get_collision_height() const override { return 1.75f; }
	float_t get_collision_depth() const override { return 0.5f; }

	float_t get_step_speed() const { return step_speed_; }

	float_t get_last_footing_height() const { return last_footing_height_; }

	void restart() override;

	/// 更新
	void update() override;
	
	// 移動
	void step( float_t );
	void side_step( float_t );
	void ladder_step( float_t );

	void release_ladder();

	/// ジャンプ処理
	void jump();
	void super_jump();

	/// よじ登り
	void clamber();
	void stop_clamber();

	// 停止
	void stop();
	void stop_ladder_step();

	/// 方向加算
	void add_direction_degree( float );

	void set_direction_degree( float_t, bool = false );

	void set_pitch( float_t p ) { pitch_ = p; }
	bool is_ladder_step_only() const;

	void rocket( const Vector3& );
	void finish_rocketing();

	void throw_stone( const Vector3& );
	void start_flickering() { is_flickering_ = true; action_base_position_ = get_location(); }
	void stop_flickering() { is_flickering_ = false; }


	void damage( const Vector3& );
	void kill();

	void set_eye_depth( float );
	void add_eye_depth( float );

	float_t get_eye_height() const { return eye_height_; }
	float_t get_eye_depth() const { return eye_depth_; }

	//
	bool is_falling() const;
	bool is_located_on_die() const { return is_located_on_die_; }
	bool is_located_on_safe() const { return is_located_on_safe_; }
	bool is_falling_to_die() const { return is_falling() && ! is_on_ladder() && ! is_rocketing() && is_located_on_die(); }
	
	bool is_on_footing() const { return is_on_footing_; }
	bool is_on_ladder() const { return is_on_ladder_; }
	bool is_jumpable() const { return is_jumpable_; }
	bool is_jumping() const { return is_jumping_; }
	bool is_clambering() const { return is_clambering_; }
	bool is_facing_to_block() const { return is_facing_to_block_; }
	bool can_clamber() const { return can_clamber_; }
	bool can_peer_down() const { return can_peer_down_; }
	bool can_throw() const { return can_throw_; }
	bool is_rocketing() const { return action_mode_ == ACTION_MODE_ROCKET; }
	bool is_flickering() const { return is_flickering_; }
	bool can_running() const;

	bool has_medal() const { return has_medal_; }

	ActionMode get_action_mode() const { return action_mode_; }
	void set_action_mode( ActionMode );

	bool is_action_pre_finish() const { return is_action_pre_finish_; }

	ItemType get_selected_item_type() const { return selected_item_type_; }
	int get_item_count( ItemType ) const;

	void select_prev_item();
	void select_next_item();

	void switch_scope_mode();

	int_t pop_balloon_sound_request() { int_t r = balloon_sound_request_; balloon_sound_request_ = 0; return r; }

	const Balloon* get_balloon() const { return balloon_; }

	int get_hp() const { return hp_; }

}; // class Player

} // namespace blue_sky

#endif // BLUE_SKY_PLAYER_H
