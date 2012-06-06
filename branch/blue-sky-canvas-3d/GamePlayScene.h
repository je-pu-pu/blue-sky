#ifndef BLUE_SKY_GAME_PLAY_SCENE_H
#define BLUE_SKY_GAME_PLAY_SCENE_H

#include "Scene.h"
#include "vector3.h"

#include <common/safe_ptr.h>

class Direct3D9Font;
class Direct3D9Mesh;
class Direct3D9SkyBox;
class Direct3D9Box;
class Direct3D9Rectangle;

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

	common::safe_ptr< Direct3D9Mesh > player_mesh_;
	common::safe_ptr< Direct3D9Mesh > goal_mesh_;
	common::safe_ptr< Direct3D9Mesh > shadow_mesh_;
	common::safe_ptr< Direct3D9Mesh > ground_mesh_;
	common::safe_ptr< Direct3D9Mesh > scope_mesh_;

	common::safe_ptr< Direct3D9Mesh > balloon_mesh_;
	common::safe_ptr< Direct3D9Mesh > rocket_mesh_;
	common::safe_ptr< Direct3D9Mesh > umbrella_mesh_;
	common::safe_ptr< Direct3D9Mesh > aim_mesh_;

	common::safe_ptr< Direct3D9SkyBox > sky_box_;

	common::safe_ptr< Direct3D9Box > box_;

	common::safe_ptr< Direct3D9Rectangle > rectangle_;

	LPDIRECT3DTEXTURE9 back_buffer_texture_;
	LPDIRECT3DSURFACE9 back_buffer_surface_;
	LPDIRECT3DSURFACE9 depth_surface_;

	Texture* ui_texture_;

	float brightness_;
	bool is_cleared_;

	float ambient_color_[ 4 ];
	float grid_object_visible_length_;
	float grid_object_lod_0_length_;

	enum LensType
	{
		LENS_TYPE_NORMAL = 0,
		LENS_TYPE_FISH_EYE = 1,
		LENS_TYPE_CRAZY
	};

	LensType lens_type_;

	void generate_random_stage();

	void load_stage_file( const char* );
	void save_stage_file( const char* ) const;

public:
	GamePlayScene( const GameMain* );
	~GamePlayScene();

	void update();				///< メインループ
	bool render();				///< 描画

	void render_shadow( const ActiveObject*, const D3DXMATRIXA16& );

	bool is_clip_cursor_required() { return true; }

}; // class GamePlayScene

} // namespace blue_sky

#endif // BLUE_SKY_GAME_PLAY_SCENE_H