#ifndef BLUE_SKY_ROBOT_H
#define BLUE_SKY_ROBOT_H

#include "ActiveObject.h"
#include <game/Texture.h>

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
	typedef game::Texture Texture;

	enum Mode
	{
		MODE_STAND = 0,
		MODE_ATTENTION,
		MODE_FIND,
		MODE_CHASE,
		MODE_SHUTDOWN,
		MODE_FLOAT,
		// MODE_DETOUR
	};

private:
	const Player*	player_;
	const Texture*	texture_;		///< �e�N�X�`��

	Mode			mode_;			///< ���݂̓��샂�[�h
	float_t			timer_;			///< �ėp�^�C�}�[

	void on_collide_with( GameObject* o ) { o->on_collide_with( this ); }
	void on_collide_with( Player* );
	void on_collide_with( Stone* );

protected:
	bool caluclate_target_visible() const;
	bool caluclate_target_lost() const;

public:
	Robot();
	~Robot() { }

	void set_player( const Player* p ) { player_ = p; }

	void restart();

	/// �X�V
	void update();

	Mode get_mode() const { return mode_; };
	
	float get_collision_width() const { return 1.f; }
	float get_collision_height() const { return 2.f; }
	float get_collision_depth() const  { return 0.5f; }

	void render_material_at( uint_t ) const override;

	void shutdown();
	void start_floating();

}; // class Robot

} // namespace blue_sky

#endif // BLUE_SKY_ROBOT_H
