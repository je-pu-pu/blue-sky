#ifndef BLUE_SKY_GAME_PLAY_SCENE_H
#define BLUE_SKY_GAME_PLAY_SCENE_H

#include "Scene.h"

#include <common/auto_ptr.h>

class Direct3D11Mesh;
class Direct3D11FarBillboardsMesh;
class Direct3D11SkyBox;
class Direct3D11ShadowMap;
class Direct3D11Rectangle;
class Direct3D11Axis;

namespace blue_sky
{

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
	typedef Direct3D11Axis				Axis;

protected:
	float_t				object_detail_level_0_length_;			///< 低品質のオブジェクトを表示する距離
	float_t				object_detail_level_1_length_;			///< 中品質のオブジェクトを表示する距離
	float_t				object_detail_level_2_length_;			///< 高品質のオブジェクトを表示する距離

	Texture*			ui_texture_;							///< UI 表示用テクスチャ

	bool				is_cleared_;							///< ステージクリアフラグ

	mutable common::auto_ptr< ShadowMap >		shadow_map_;
	common::auto_ptr< SkyBox >			sky_box_;
	common::auto_ptr< Mesh >			ground_;
	common::auto_ptr< Mesh >			far_billboards_;
	common::auto_ptr< Rectangle >		rectangle_;

	common::auto_ptr< Axis >			debug_axis_;

	common::auto_ptr< Stage >			stage_;
	common::auto_ptr< Goal >			goal_;

	common::auto_ptr< Player >			player_;
	common::auto_ptr< Camera >			camera_;

	Sound*								bgm_;
	Sound*								balloon_bgm_;
	float_t								action_bgm_after_timer_;

	float_t								bpm_;

	void generate_random_stage();

	void load_stage_file( const char* );
	void save_stage_file( const char* ) const;

	void render_sprite();

	void restart();

	void on_goal();

	void update_main();
	void update_clear();

	void set_bpm( const float_t bpm ) { bpm_ = bpm; }
	float_t get_bpm() const { return bpm_; }

	/** @todo 移動する */
	void update_render_data_for_game() const;
	void update_render_data_for_frame() const;
	void update_render_data_for_object() const;
	void update_render_data_for_active_object( const ActiveObject* ) const;

	void bind_game_constant_buffer() const;
	void bind_frame_constant_buffer() const;
	void bind_object_constant_buffer() const;

	void render_text() const;

	void render_shadow_map() const;
	void render_shadow_map( const char*, bool ) const;
	
	void render_sky_box() const;
	void render_far_billboards() const;

	void render_object_skin_mesh() const;
	void render_object_mesh() const;
	void render_object_line() const;
	
	void render_active_object_mesh( const ActiveObject* ) const;
	void render_active_object_line( const ActiveObject* ) const;

	void render_debug_axis() const;
	void render_debug_bullet() const;
	void render_debug_shadow_map_window() const;



public:
	GamePlayScene( const GameMain* );
	~GamePlayScene();

	void update();				///< メインループ
	void render();				///< 描画

	bool is_clip_cursor_required() { return true; }

}; // class GamePlayScene

} // namespace blue_sky

#endif // BLUE_SKY_GAME_PLAY_SCENE_H