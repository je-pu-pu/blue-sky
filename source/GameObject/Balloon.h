#ifndef BLUE_SKY_BALLOON_H
#define BLUE_SKY_BALLOON_H

#include "ActiveObject.h"

namespace blue_sky
{

/**
 * ïóëD
 *
 */
class Balloon : public ActiveObject
{
public:
	const Player* player_;
	float flicker_;

private:
	float_t get_collision_width() const override { return 1.5f; }
	float_t get_collision_height() const override { return 1.5f; }
	float_t get_collision_depth() const override { return 1.5f; }
	
	float_t get_height_offset() const override { return 0.f; }

	void on_collide_with( GameObject* o ) override { o->on_collide_with( this ); }
	void on_collide_with( Stone* ) override;

	bool is_hard() const override { return false; }
	bool is_safe_footing() const override { return true; }

	void kill() override;

public:
	Balloon();
	~Balloon() { }

	virtual void set_drawing_model( const DrawingModel* m ) override;

	/// çXêV
	void update() override;

	void restart() override;
	
	void set_player( const Player* p ) { player_ = p; flicker_ = 0.f; }
	const Player* get_player() const { return player_; }

	bool is_mesh_visible() const override;
	bool is_line_visible() const override;

	bool is_visible_with_player() const;

}; // class Balloon

} // namespace blue_sky

#endif // BLUE_SKY_BALLOON_H
