#ifndef BLUE_SKY_GAME_PLAY_SCENE_H
#define BLUE_SKY_GAME_PLAY_SCENE_H

#include "Scene.h"
#include "vector3.h"

#include <common/safe_ptr.h>

class Direct3D9Font;
class Direct3D9Mesh;
class Direct3D9SkyBox;
class Direct3D9Box;

namespace blue_sky
{

class Player;
class Goal;
class ActiveObject;
class Camera;
class Stage;

/**
 * ゲームプレイ中の処理を管理する
 *
 */
class GamePlayScene : public Scene
{
protected:
	common::safe_ptr< Player > player_;
	common::safe_ptr< Goal > goal_;

	common::safe_ptr< Camera > camera_;
	common::safe_ptr< Stage > stage_;

	common::safe_ptr< Direct3D9Font > font_;
	common::safe_ptr< Direct3D9Mesh > player_mesh_;
	common::safe_ptr< Direct3D9Mesh > goal_mesh_;
	common::safe_ptr< Direct3D9Mesh > enemy_mesh_;
	common::safe_ptr< Direct3D9Mesh > shadow_mesh_;
	common::safe_ptr< Direct3D9Mesh > ground_mesh_;

	common::safe_ptr< Direct3D9SkyBox > sky_box_;

	common::safe_ptr< Direct3D9Box > box_;

	float ambient_color_[ 4 ];
	float grid_object_visible_length_;
	float grid_object_lod_0_length_;

	void generate_random_stage();

	void load_stage_file( const char* );
	void save_stage_file( const char* ) const;

public:
	GamePlayScene( const GameMain*, const std::string& );
	~GamePlayScene();

	void update();				///< メインループ
	bool render();				///< 描画

	void render_shadow( const ActiveObject*, const D3DXMATRIXA16& );

	bool is_clip_cursor_required() { return true; }

}; // class GamePlayScene

} // namespace blue_sky

#endif // BLUE_SKY_GAME_PLAY_SCENE_H