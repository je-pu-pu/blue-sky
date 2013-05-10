#ifndef BLUE_SKY_ROBOT_H
#define BLUE_SKY_ROBOT_H

#include "ActiveObject.h"

#include <vector>

namespace blue_sky
{

class Player;

/**
 * �G
 *
 */
class Robot : public ActiveObject
{
public:
	enum Mode
	{
		MODE_STAND = 0,
		MODE_FIND,
		MODE_CHASE,
		// MODE_DETOUR
	};

	typedef std::vector< const DrawingModel* > DrawingModelList;

private:
	const Player*	player_;

	Mode			mode_;
	float_t			timer_;

	DrawingModelList drawing_model_list_;

	void on_collide_with( ActiveObject* o ) { o->on_collide_with( this ); }

protected:

public:
	Robot();
	~Robot() { }

	void set_player( const Player* p ) { player_ = p; }

	void restart();

	/// �X�V
	void update();
	
	float get_collision_width() const { return 1.f; }
	float get_collision_height() const { return 2.f; }
	float get_collision_depth() const  { return 0.5f; }

	void add_drawing_model( const DrawingModel* );


}; // class Robot

} // namespace blue_sky

#endif // BLUE_SKY_ROBOT_H