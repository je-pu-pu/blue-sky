#ifndef BLUE_SKY_GAME_PLAY_SCENE_H
#define BLUE_SKY_GAME_PLAY_SCENE_H

#include "Scene.h"

#include <common/auto_ptr.h>

class Direct3D11Mesh;
class Direct3D11FarBillboardsMesh;
class Direct3D11SkyBox;
class Direct3D11ShadowMap;
class Direct3D11Rectangle;

namespace blue_sky
{

class Player;
class Goal;
class ActiveObject;
class Camera;
class Stage;
class DrawingModel;

/**
 * �Q�[���v���C���̏������Ǘ�����
 *
 */
class GamePlayScene : public Scene
{
public:
	typedef Direct3D11Mesh				Mesh;
	typedef Direct3D11FarBillboardsMesh	FarBillboardsMesh;
	typedef Direct3D11SkyBox			SkyBox;
	typedef Direct3D11ShadowMap			ShadowMap;
	typedef Direct3D11Rectangle			Rectangle;

protected:
	float_t				object_detail_level_0_length_;			///< ��i���̃I�u�W�F�N�g��\�����鋗��
	float_t				object_detail_level_1_length_;			///< ���i���̃I�u�W�F�N�g��\�����鋗��
	float_t				object_detail_level_2_length_;			///< ���i���̃I�u�W�F�N�g��\�����鋗��

	Texture*			ui_texture_;							///< UI �\���p�e�N�X�`��

	bool				is_cleared_;							///< �X�e�[�W�N���A�t���O

	common::auto_ptr< ShadowMap >		shadow_map_;
	common::auto_ptr< SkyBox >			sky_box_;
	common::auto_ptr< Mesh >			ground_;
	common::auto_ptr< Mesh >			far_billboards_;
	common::auto_ptr< Rectangle >		rectangle_;

	common::auto_ptr< Stage >			stage_;
	common::auto_ptr< Goal >			goal_;

	common::auto_ptr< Player >			player_;
	common::auto_ptr< Camera >			camera_;

	Sound*								bgm_;
	Sound*								balloon_bgm_;
	float_t								action_bgm_after_timer_;

	void generate_random_stage();

	void load_stage_file( const char* );
	void save_stage_file( const char* ) const;

	void render_sprite();

	void render( const ActiveObject* );
	void render_line( const ActiveObject* );

	void restart();

	void on_goal();

	void update_main();
	void update_clear();

public:
	GamePlayScene( const GameMain* );
	~GamePlayScene();

	void update();				///< ���C�����[�v
	void render();				///< �`��

	bool is_clip_cursor_required() { return true; }

}; // class GamePlayScene

} // namespace blue_sky

#endif // BLUE_SKY_GAME_PLAY_SCENE_H