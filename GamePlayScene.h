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
	common::safe_ptr< Camera > camera_;
	common::safe_ptr< Stage > stage_;

	common::safe_ptr< Direct3D9Font > font_;
	common::safe_ptr< Direct3D9Mesh > player_mesh_;
	common::safe_ptr< Direct3D9Mesh > shadow_mesh_;
	common::safe_ptr< Direct3D9Mesh > ground_mesh_;

	common::safe_ptr< Direct3D9SkyBox > sky_box_;

	common::safe_ptr< Direct3D9Box > box_;

	vector3 player_start_position_;

	int panorama_y_division_;
	int get_panorama_y_division() const { return panorama_y_division_; }

	void generate_random_stage();

	void load_stage_file( const char* );
	void save_stage_file( const char* ) const;

public:
	GamePlayScene( const GameMain* );
	~GamePlayScene();

	void update();				///< メインループ
	void render();				///< 描画

}; // class Scene

} // namespace GamePlayScene

#endif // BLUE_SKY_SCENE_H