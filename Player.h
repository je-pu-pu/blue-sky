#ifndef BLUE_SKY_PLAYER_H
#define BLUE_SKY_PLAYER_H

#include "ActiveObject.h"

namespace blue_sky
{

/**
 * プレイヤー
 *
 */
class Player : public ActiveObject
{
public:


private:
	const Input* input_;			///< 入力への参照

	float		step_speed_;		///< 移動速度

	float		eye_height_;		///< 目の高さ

	bool		is_dead_;			///< 死亡フラグ
	bool		is_turn_avaiable_;	///< 方向転換有効フラグ
	bool		is_jumping_;		///< ジャンプ中フラグ
	bool		is_clambering_;		///< よじ登り中フラグ

	bool		is_falling_;		///< 落下中フラグ
	vector3		velocity_on_fall_;	///< 落下開始時の移動量
	
	void on_collision_x( const GridCell& );
	void on_collision_y( const GridCell& );
	void on_collision_z( const GridCell& );

	float get_collision_width() const;
	float get_collision_height() const;
	float get_collision_depth() const;

public:
	Player();
	~Player() { }
	
	void add_direction_degree( float );

	float get_eye_height() const { return eye_height_; }

	void step( float );
	void side_step( float );

	bool is_dead() const { return is_dead_; }
	void rebirth();

	bool is_turn_available() const { return is_turn_avaiable_; }

	float get_step_speed() const { return step_speed_; }
	float get_side_step_speed() const { return 0.004f; }

	/// 更新
	void update();
	
	/// ジャンプ処理
	void jump();
	bool is_jumping() const { return is_jumping_; }

	bool is_clambering() const { return is_clambering_; }

	/// 落下処理
	void fall();
	bool is_falling() const { return is_falling_; }
	bool is_falling_to_dead() const;

	void set_input( const Input* );
	
}; // class Player

} // namespace blue_sky

#endif // BLUE_SKY_PLAYER_H
