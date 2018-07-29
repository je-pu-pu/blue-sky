#pragma once

#include "Scene.h"

#include <common/chase_value.h>
#include <common/auto_ptr.h>
#include <common/safe_ptr.h>

#include <map>
#include <list>
#include <vector>
#include <functional>

class Direct3D11ShadowMap;

namespace game
{
	class Mesh;
	class Shader;
	class Config;
};

namespace blue_sky
{
	class Player;
	class Girl;
	class Goal;
	class ActiveObject;
	class Camera;
	class DrawingModel;
	class DelayedCommand;

	struct FrameConstantBufferData;

	namespace graphics
	{
		class Rectangle;
		class BypassShader;
	}

using graphics::BypassShader;

/**
 * ゲームプレイ中の処理を管理する
 *
 */
class GamePlayScene : public Scene
{
public:
	typedef game::Mesh					Mesh;
	typedef game::Shader				Shader;
	typedef graphics::Rectangle			Rectangle;
	typedef Direct3D11ShadowMap			ShadowMap;	

	typedef std::function< void( string_t ) > Command;
	typedef std::map< string_t, Command > CommandMap;

	typedef std::function< void() > CommandCall;
	typedef std::list< CommandCall > CommandCallList;
	typedef std::list< DelayedCommand* > DelayedCommandList;

	enum BalloonSoundType
	{
		BALLOON_SOUND_TYPE_NONE = 0,		// サウンドなし
		BALLOON_SOUND_TYPE_MIX,				// サウンドを BGM とミックス
		BALLOON_SOUND_TYPE_SOLO,			// BGM を消してサウンドのみ再生
		BALLOON_SOUND_TYPE_SCALE,			// BGM を消して音階を再生
	};

protected:
	Texture*							ui_texture_ = 0;		///< UI 表示用テクスチャ
	bool								is_cleared_ = false;	///< ステージクリアフラグ
	common::auto_ptr< Config >			stage_config_;	

	mutable common::auto_ptr< ShadowMap >		shadow_map_;
	Shader*								shadow_map_shader_;
	Shader*								shadow_map_skin_shader_;

	common::auto_ptr< Player >			player_;
	common::auto_ptr< Camera >			camera_;
	common::safe_ptr< Girl >			girl_;
	Goal*								goal_ = 0;

	/// @todo 整理する
	Model*								far_billboards_ = 0;	///< 遠景ビルボード
	Model*								scope_mesh_ = 0;		///< 双眼鏡
	Model*								rectangle_;
	BypassShader*						rectangle_shader_;

	Sound*								bgm_ = 0;
	common::safe_ptr< Sound >			balloon_bgm_;
	float_t								action_bgm_after_timer_;

	float_t								bpm_;

	/// @todo GraphicsManager に移行する
	float_t								drawing_accent_scale_;	///< ( 0 : アクセントを完全に無効化 ) .. ( 2 : アクセント 2 倍 ) ..
	
	common::chase_value< Vector3, float_t >		light_position_;
	common::chase_value< Color, float_t >		ambient_color_;
	common::chase_value< Color, float_t >		shadow_color_;
	common::chase_value< Color, float_t >		shadow_paper_color_;
	bool										shading_enabled_;

	BalloonSoundType					balloon_sound_type_;

	CommandMap							command_map_;
	CommandCallList						stage_setup_command_call_list_;
	DelayedCommandList					delayed_command_list_;

	bool								is_blackout_;
	float_t								blackout_timer_;

	void load_stage_file( const char* );
	void save_stage_file( const char* ) const;

	void load_sound_all( bool );
	void setup_stage();
	
	void setup_command();
	
	// commands
	ActiveObject* create_object_at_player_front( const char_t* );

	void restart();

	void on_goal();

	void set_bpm( const float_t bpm ) { bpm_ = bpm; }
	float_t get_bpm() const { return bpm_; }

	void update_main();
	void update_clear();
	void update_blackout();
	void go_to_next_scene();

	void update_delayed_command();
	void clear_delayed_command();

	void update_balloon_sound();
	void update_shadow();
	
	void exec_command( const string_t& );

	/** @todo 移動する */
	void update_render_data_for_frame() const;
	void update_render_data_for_frame_drawing() const;
	void update_render_data_for_frame_for_eye( int ) const;
	void update_render_data_for_object() const;

	void update_frame_constant_buffer_data_sub( FrameConstantBufferData& ) const;

	void render_to_oculus_vr() const;
	void render_to_display() const;

	void render_for_eye( float_t ortho_offset = 0.f ) const;

	void render_text() const;

	void render_shadow_map() const;
	void render_shadow_map( const Shader* , bool ) const;
	
	void render_sky_box() const;
	void render_far_billboards() const;

	void render_object_mesh() const;
	void render_object_line() const;
	
	void render_sprite( float_t rotho_offset = 0.f ) const;

	void render_debug_axis() const;
	void render_debug_axis_for_bones( const ActiveObject* ) const;

	/// @todo GraphicsManager に移行する
	void render_debug_shadow_map_window() const;

public:
	explicit GamePlayScene( const GameMain* );
	~GamePlayScene();

	void update() override;				///< メインループ
	void render() override;				///< 描画

	bool is_clip_cursor_required() override { return true; }

}; // class GamePlayScene

} // namespace blue_sky
