#pragma once

#include "Scene.h"

#include <common/chase_value.h>

#include <list>
#include <memory>
#include <unordered_map>
#include <functional>

namespace game
{
	class Config;
};

namespace core::graphics
{
	class Mesh;
	class Shader;
}

namespace blue_sky
{
	class Player;
	class Girl;
	class Goal;
	class ActiveObject;
	class Camera;
	class DelayedCommand;

	struct FrameConstantBufferData;

	namespace graphics
	{
		class Rectangle;
	}

/**
 * ゲームプレイ中の処理を管理する
 *
 */
class GamePlayScene : public Scene
{
public:
	static inline const char_t* name = "game_play";

	using Mesh					= core::graphics::Mesh;
	using Shader				= core::graphics::Shader;
	using Rectangle				= graphics::Rectangle;
	
	using Command				= std::function< void( string_t ) >;
	using CommandMap			= std::unordered_map< string_t, Command >;

	using CommandCall			= std::function< void() >;
	using CommandCallList		= std::list< CommandCall >;
	using DelayedCommandList	= std::list< std::unique_ptr< DelayedCommand > >;

	enum class BalloonSoundType
	{
		NONE = 0,				// サウンドなし
		MIX,					// サウンドを BGM とミックス
		SOLO,					// BGM を消してサウンドのみ再生
		SCALE,					// BGM を消して音階を再生
	};

private:
	// 非所有参照 (各 Manager が所有)
	Texture*							ui_texture_ = nullptr;			///< UI テクスチャ (GraphicsManager)
	Shader*								debug_texture_shader_ = nullptr;///< デバッグシェーダー (GraphicsManager)
	Model*								far_billboards_ = nullptr;		///< 遠景ビルボード (GraphicsManager)
	Model*								scope_mesh_ = nullptr;			///< 双眼鏡 (GraphicsManager)
	Model*								rectangle_ = nullptr;			///< 矩形モデル (GraphicsManager)
	Sound*								bgm_ = nullptr;					///< BGM (SoundManager)

	// 非所有参照 (ActiveObjectManager が所有)
	Player*								player_ = nullptr;
	Girl*								girl_ = nullptr;
	Goal*								goal_ = nullptr;

	// 所有リソース
	std::unique_ptr< Config >			stage_config_;
	std::unique_ptr< Camera >			camera_;

	bool								is_cleared_ = false;	///< ステージクリアフラグ
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

protected:
	// Timing constants
	float_t get_blackout_timeout() const { return 6.f; }
	float_t get_action_bgm_fade_delay() const { return 2.f; }
	float_t get_mouse_rotation_speed() const { return 90.f; }
	float_t get_scope_zoom_factor() const { return 0.5f; }
	float_t get_min_scope_fov() const { return 5.f; }
	float_t get_camera_chase_speed_on_clear() const { return 0.1f; }

	// Fade speeds
	float_t get_fade_speed_falling() const { return 0.01f; }
	float_t get_fade_speed_normal() const { return 0.05f; }
	float_t get_fade_speed_dead() const { return 0.1f; }
	float_t get_fade_speed_clear() const { return 0.0025f; }

	// Goal camera
	float_t get_goal_camera_z_offset() const { return 4.f; }
	float_t get_goal_camera_z_speed() const { return 0.5f; }
	float_t get_goal_position_lerp() const { return 0.05f; }

	// Door sound timing
	float_t get_door_sound_start() const { return 6.f; }
	float_t get_door_sound_end() const { return 8.f; }

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

	void update_frame_constant_buffer_data_sub( FrameConstantBufferData& ) const;

	void render_to_oculus_vr() const;
	void render_to_display() const;

	void render_for_eye( float_t ortho_offset = 0.f ) const;

	void render_text() const;

	/// @todo GraphicsManager に移行する
	void render_far_billboards() const;
	
	void render_sprite( float_t rotho_offset = 0.f ) const;

	void render_debug_axis_for_bones( const ActiveObject* ) const;

	/// @todo GraphicsManager に移行する
	void render_debug_shadow_map_window() const;

public:
	explicit GamePlayScene();
	~GamePlayScene();

	void update() override;				///< メインループ
	void render() override;				///< 描画

	bool is_clip_cursor_required() override { return true; }

}; // class GamePlayScene

} // namespace blue_sky
