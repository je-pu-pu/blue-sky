#ifndef BLUE_SKY_ROBOT_H
#define BLUE_SKY_ROBOT_H

#include "ActiveObject.h"

#include <vector>

namespace blue_sky
{

class Player;

/**
 * 敵
 *
 */
class Robot : public ActiveObject
{
public:
	enum Mode
	{
		MODE_STAND = 0,
		MODE_ATTENTION,
		MODE_FIND,
		MODE_CHASE,
		MODE_SHUTDOWN,
		// MODE_DETOUR
	};

private:
	const Player*	player_;

	Mode			mode_;			///< 現在の動作モード
	float_t			timer_;			///< 汎用タイマー

	void on_collide_with( GameObject* o ) { o->on_collide_with( this ); }
	void on_collide_with( Player* );

protected:
	bool caluclate_target_visible() const;
	bool caluclate_target_lost() const;


public:
	Robot();
	~Robot() { }

	void set_player( const Player* p ) { player_ = p; }

	void restart();

	/// 更新
	void update();

	Mode get_mode() const { return mode_; };
	
	float get_collision_width() const { return 1.f; }
	float get_collision_height() const { return 2.f; }
	float get_collision_depth() const  { return 0.5f; }

}; // class Robot

} // namespace blue_sky

#endif // BLUE_SKY_ROBOT_H
