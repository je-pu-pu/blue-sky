#ifndef BLUE_SKY_GAME_PLAY_SCENE_H
#define BLUE_SKY_GAME_PLAY_SCENE_H

#include "Scene.h"

class Direct3D9Mesh;
class Direct3D9SkyBox;
class Direct3D9Box;

namespace blue_sky
{

class Player;
class Camera;
class Stage;
class Building;
class House;

/**
 * ゲームプレイ中の処理を管理する
 *
 */
class GamePlayScene : public Scene
{
protected:
	Player* player_;
	Camera* camera_;
	Stage* stage_;
	Building* building_a_grid_;
	House* house_a_grid_;

	Direct3D9Mesh* building_a_mesh_;
	Direct3D9Mesh* house_a_mesh_;
	Direct3D9Mesh* shadow_mesh_;
	Direct3D9Mesh* ground_mesh_;

	Direct3D9SkyBox* sky_box_;

	Direct3D9Box* box_;

	int panorama_y_division_;
	int get_panorama_y_division() const { return panorama_y_division_; }

public:
	GamePlayScene( const GameMain* );
	~GamePlayScene();

	void update();				///< メインループ
	void render();				///< 描画

}; // class Scene

} // namespace GamePlayScene

#endif // BLUE_SKY_SCENE_H