#ifndef BLUE_SKY_GAME_PLAY_SCENE_H
#define BLUE_SKY_GAME_PLAY_SCENE_H

#include "Scene.h"
#include "Direct3D11Vector.h"
#include "Direct3D11Color.h"

#include "vector3.h"

#include <common/chase_value.h>
#include <common/auto_ptr.h>
#include <common/safe_ptr.h>

#include <map>
#include <list>
#include <vector>
#include <functional>

class Direct3D11FarBillboardsMesh;
class Direct3D11SkyBox;
class Direct3D11ShadowMap;
class Direct3D11Rectangle;
class Direct3D11Axis;

namespace game
{

class Mesh;
class Config;

}; // namespace game

namespace blue_sky
{

class Player;
class Girl;
class Goal;
class ActiveObject;
class Camera;
class DrawingModel;

/**
 * ゲームプレイ中の処理を管理する
 *
 */
class GamePlayScene : public Scene
{
public:
	typedef game::Mesh					Mesh;
	typedef Direct3D11FarBillboardsMesh	FarBillboardsMesh;
	typedef Direct3D11SkyBox			SkyBox;
	typedef Direct3D11ShadowMap			ShadowMap;
	typedef Direct3D11Rectangle			Rectangle;
	typedef Direct3D11Axis				Axis;

	typedef std::function< void( string_t ) > Command;
	typedef std::map< string_t, Command > CommandMap;

	typedef std::function< void() > CommandCall;
	typedef std::list< CommandCall > CommandCallList;

protected:
	Texture*			ui_texture_;							///< UI 表示用テクスチャ
	bool				is_cleared_;							///< ステージクリアフラグ
	common::auto_ptr< Config >			stage_config_;	

	mutable common::auto_ptr< ShadowMap >		shadow_map_;
	common::auto_ptr< SkyBox >			sky_box_;
	common::auto_ptr< Mesh >			ground_;
	common::auto_ptr< Mesh >			far_billboards_;
	common::auto_ptr< Rectangle >		rectangle_;

	common::auto_ptr< Axis >			debug_axis_;

	common::auto_ptr< Goal >			goal_;

	common::auto_ptr< Player >			player_;
	common::auto_ptr< Camera >			camera_;

	common::safe_ptr< Girl >			girl_;

	common::auto_ptr< Mesh >			scope_mesh_;			///< 双眼鏡

	Sound*								bgm_;
	common::safe_ptr< Sound >			balloon_bgm_;
	float_t								action_bgm_after_timer_;

	float_t								bpm_;

	std::vector< Texture* >				paper_texture_list_;
	Texture*							paper_texture_;
	uint_t								drawing_line_type_index_;
	float_t								drawing_accent_scale_;	///< ( 0 : アクセントを完全に無効化 ) .. ( 2 : アクセント 2 倍 ) ..
	
	common::chase_value< vector3, float_t >		light_position_;
	common::chase_value< Color, float_t >		ambient_color_;
	common::chase_value< Color, float_t >		shadow_color_;
	common::chase_value< Color, float_t >		shadow_paper_color_;

	CommandMap							command_map_;
	CommandCallList						stage_setup_command_call_list_;

	void load_stage_file( const char* );
	void save_stage_file( const char* ) const;

	void load_sound_all( bool );
	void setup_stage();
	void setup_command();

	void render_sprite();

	void restart();

	void on_goal();

	void set_bpm( const float_t bpm ) { bpm_ = bpm; }
	float_t get_bpm() const { return bpm_; }

	void update_main();
	void update_clear();

	void update_shadow();

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