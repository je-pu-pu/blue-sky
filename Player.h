#ifndef BLUE_SKY_PLAYER_H
#define BLUE_SKY_PLAYER_H

#include "vector3.h"

namespace blue_sky
{

/**
 * プレイヤー
 *
 */
class Player
{
private:
	vector3 position_;	///< 座標
	vector3 velocity_;	///< 移動
	
	bool jumping_;	// ジャンプ中フラグ
	float floor_height_; ///< 足場の高さ
	
	float max_speed();
public:
	Player();
	~Player() { }
	
	vector3& position() { return position_; }
	vector3& velocity() { return velocity_; }

	/// 更新
	void update();
	
	/// ジャンプ処理
	void jump();

	bool jumping() { return jumping_; }

	float get_floor_height() const { return floor_height_; }
	void set_floor_height( float h ) { floor_height_ = h; }
	
}; // class Player

} // namespace blue_sky

#endif // BLUE_SKY_PLAYER_H
