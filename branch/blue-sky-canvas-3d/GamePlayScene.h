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

using common::auto_ptr;

class Player;
class Goal;
class ActiveObject;
class Camera;
class Stage;
class DrawingModel;

/**
 * ゲームプレイ中の処理を管理する
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
	float_t				object_detail_level_0_length_;			///< 低品質のオブジェクトを表示する距離
	float_t				object_detail_level_1_length_;			///< 中品質のオブジェクトを表示する距離
	float_t				object_detail_level_2_length_;			///< 高品質のオブジェクトを表示する距離

	Texture*			ui_texture_;							///< UI 表示用テクスチャ

	bool				is_cleared_;							///< ステージクリアフラグ

	auto_ptr< ShadowMap >				shadow_map_;
	auto_ptr< SkyBox >					sky_box_;
	auto_ptr< Mesh >					ground_;
	auto_ptr< Mesh >					far_billboards_;
	auto_ptr< Rectangle >				rectangle_;

	auto_ptr< Stage >					stage_;
	auto_ptr< Goal >					goal_;

	auto_ptr< Player >					player_;
	auto_ptr< Camera >					camera_;

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

	void update();				///< メインループ
	void render();				///< 描画

	bool is_clip_cursor_required() { return true; }

}; // class GamePlayScene

} // namespace blue_sky

#endif // BLUE_SKY_GAME_PLAY_SCENE_H