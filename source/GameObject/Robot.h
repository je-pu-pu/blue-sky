#pragma once

#include "ActiveObject.h"
#include <game/Texture.h>

#include <vector>
#include <list>

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
	typedef game::Texture Texture;
	typedef btAlignedObjectArray< Vector3 > Vector3Array;

	enum Mode
	{
		MODE_STAND = 0,
		MODE_ROTATION,
		MODE_PATROL,
		MODE_ATTENTION,
		MODE_FIND,
		MODE_CHASE,
		MODE_SHUTDOWN,
		MODE_FLOAT,
		// MODE_DETOUR
	};

private:
	const Player*	player_;
	const Texture*	texture_;		///< テクスチャ

	Mode			mode_;			///< 現在の動作モード
	Mode			mode_backup_;	///< 前回プレイヤーを発見した時の動作モード
	float_t			timer_;			///< 汎用タイマー

	Vector3Array	patrol_point_list_;	///< 巡回ポイントの一覧
	int_t			current_patrol_point_index_;

	void on_collide_with( GameObject* o ) { o->on_collide_with( this ); }
	void on_collide_with( Player* );
	void on_collide_with( Stone* );

protected:
	bool caluclate_target_visible() const;
	bool caluclate_target_lost() const;

	bool caluclate_collide_object_to_swtich_off( const GameObject* );

	void update_patrol();

public:
	Robot();
	~Robot() { }

	void set_player( const Player* p ) { player_ = p; }

	void restart() override;

	/// 更新
	void update() override;

	void action( const string_t& ) override;

	Mode get_mode() const { return mode_; };
	
	float get_collision_width() const override { return 1.f; }
	float get_collision_height() const override { return 2.f; }
	float get_collision_depth() const  override { return 0.5f; }

	void render_material_at( uint_t ) const override;

	void shutdown();
	void start_floating();

	void add_patrol_point( const Vector3& point );

}; // class Robot

} // namespace blue_sky
