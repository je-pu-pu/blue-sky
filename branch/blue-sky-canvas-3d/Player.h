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

private:
	bool is_on_footing_;		///< 現在足場上フラグ
	bool is_jumping_;			///< 現在ジャンプ中フラグ
	bool is_jumpable_;			///< 現在ジャンプ可能フラグ

	float_t uncontrollable_timer_;	///< 制御不能タイマー

	float_t eye_height_;		///< 目の高さ
	float_t eye_depth_;			///< 目の奥行きオフセット ( 身を乗り出した時に増える ) ( 0.f .. 1.f )

protected:
	const Input* get_input() const;

	float_t get_step_speed() const { return 0.25f; }
	float_t get_side_step_speed() const { return 0.25f; }

	void update_on_footing();
	void update_jumpable();
	void update_jumping();

	bool check_on_footing( float_t, float_t ) const;

	bool is_on_footing() const { return is_on_footing_; }
	bool is_jumpable() const { return is_jumpable_; }
	bool is_jumping() const { return is_jumping_; }

	bool is_uncontrollable() const { return uncontrollable_timer_ > 0.f; }

public:
	Player();
	~Player() { }

	float_t get_collision_width() const { return 0.5f; }
	float_t get_collision_height() const { return 1.75f; }
	float_t get_collision_depth() const { return 0.25f; }

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
	bool is_falling_to_dead() const;

	bool has_medal() const { return false; }

}; // class Player

} // namespace blue_sky

#endif // BLUE_SKY_PLAYER_H
