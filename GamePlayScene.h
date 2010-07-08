#ifndef BLUE_SKY_GAME_PLAY_SCENE_H
#define BLUE_SKY_GAME_PLAY_SCENE_H

#include "Scene.h"
#include "vector3.h"

class Direct3D9Mesh;
class Direct3D9SkyBox;
class Direct3D9Box;

namespace blue_sky
{

class Player;
class Camera;
class Stage;

/**
 * �Q�[���v���C���̏������Ǘ�����
 *
 */
class GamePlayScene : public Scene
{
protected:
	Player* player_;
	Camera* camera_;
	Stage* stage_;

	Direct3D9Mesh* player_mesh_;
	Direct3D9Mesh* shadow_mesh_;
	Direct3D9Mesh* ground_mesh_;

	Direct3D9SkyBox* sky_box_;

	Direct3D9Box* box_;

	vector3 player_start_position_;

	int panorama_y_division_;
	int get_panorama_y_division() const { return panorama_y_division_; }

	void generate_random_stage();

	void load_stage_file( const char* );
	void save_stage_file( const char* ) const;

public:
	GamePlayScene( const GameMain* );
	~GamePlayScene();

	void update();				///< ���C�����[�v
	void render();				///< �`��

}; // class Scene

} // namespace GamePlayScene

#endif // BLUE_SKY_SCENE_H