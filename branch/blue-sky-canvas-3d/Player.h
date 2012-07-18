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
	float get_collision_width() const { return 1.f; }
	float get_collision_height() const { return 1.75f; }
	float get_collision_depth() const { return 1.f; }

protected:
	const Input* get_input() const;

public:
	Player();
	~Player() { }

	/// 更新
	void update();
	
	/// ジャンプ処理
	void jump();

}; // class Player

} // namespace blue_sky

#endif // BLUE_SKY_PLAYER_H
