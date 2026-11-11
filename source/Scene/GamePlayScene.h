#pragma once

#include "Scene.h"
#include <blue_sky/GameObjectRef.h>
#include <blue_sky/ecs/component/PlayerInputComponent.h>
#include <blue_sky/stage/StageLook.h>
#include <blue_sky/FrameStageTimer.h>

#include <common/chase_value.h>
#include <core/setting/SettingStore.h>
#include <core/graphics/Shader.h>

#include <list>
#include <memory>
#include <unordered_map>
#include <functional>
#include <optional>

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
	class Camera;
	class DelayedCommand;

	struct FrameConstantBufferData;

	namespace graphics
	{
		class Rectangle;
	}

	namespace stage
	{
		struct Action;
	}

	namespace ecs
	{
		class EntityObject;
		class PlayerBehaviorSystem;
		struct PlayerBehaviorComponent;
		struct PlayerInventoryComponent;
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
	int debug_press_switches_frame_ = 0;		///< 開発用 : BLUE_SKY_DEBUG_PRESS_SWITCHES でスイッチを押すまでのフレーム数を数える
	int debug_visit_objects_frame_ = 0;			///< 開発用 : BLUE_SKY_DEBUG_VISIT_OBJECTS でプレイヤーを物体の所へ動かすフレームを数える（-1 で終わり）

	/// 開発用 : BLUE_SKY_DEBUG_VISIT_OBJECTS を設定すると、プレイヤーをメダル・風船・梯子・ゴールの所へ順に動かし、当たった結果をログに出す
	void debug_visit_objects();

	int debug_throw_stones_frame_ = 0;			///< 開発用 : BLUE_SKY_DEBUG_THROW_STONES で石を拾って投げるフレームを数える（-1 で終わり）

	/// 開発用 : BLUE_SKY_DEBUG_THROW_STONES を設定すると、ステージの石を全部拾い、スイッチ・敵・風船に向けて順に投げて、当たった結果をログに出す
	void debug_throw_stones();

	int debug_log_actors_frame_ = 0;			///< 開発用 : BLUE_SKY_DEBUG_LOG_ACTORS で風船・少女・敵の様子を出すフレームを数える

	/// 開発用 : BLUE_SKY_DEBUG_LOG_ACTORS を設定すると、30 フレームごとに風船・少女・敵・プレイヤーの位置・向き・状態をログに出す
	void debug_log_actors();

	int debug_player_script_frame_ = 0;			///< 開発用 : BLUE_SKY_DEBUG_PLAYER_SCRIPT で決まった入力を流すフレームを数える（-1 で終わり）

	/// 開発用 : BLUE_SKY_DEBUG_PLAYER_SCRIPT を設定すると、決まった入力の並びを流し、プレイヤーを物体の所へ置き、5 フレームごとにプレイヤーの様子をログに出す
	void debug_player_script();
	void debug_player_script_input( ecs::PlayerInputComponent& ) const;
	string_t describe_player_state() const;

	/// 開発用 : BLUE_SKY_DEBUG_SPAWN_OBJECTS="数[,種類]" を設定すると、ステージを読んだ後に、物体を格子状に大量に作る（大量の物体でのフレームの時間を測るため）
	void debug_spawn_objects();

	/// 開発用 : BLUE_SKY_DEBUG_FRAME_STAGES="最初,最後" を設定すると、update と render の段階ごとの時間と描いた回数の中央値をログに出す
	mutable FrameStageTimer stage_timer_;

	/// 入力（Input）を、プレイヤーの入力の部品（ecs::PlayerInputComponent）に書く
	void read_player_input();
	// 各 Manager が持つリソース。ハンドルで持ち、使うたびに引く（以前は生ポインタで、解放・差し替えの後に古いものを指し得た。
	// BGM はステージの bgm で同じ名前の中身が差し替わる）
	TextureHandle						ui_texture_;			///< UI テクスチャ (GraphicsManager)
	core::graphics::ShaderHandle		debug_texture_shader_;	///< デバッグシェーダー (GraphicsManager)
	ModelHandle							far_billboards_;		///< 遠景ビルボード (GraphicsManager)
	ModelHandle							scope_mesh_;			///< 双眼鏡 (GraphicsManager)
	ModelHandle							rectangle_;				///< 矩形モデル (GraphicsManager)
	SoundHandle							bgm_;					///< BGM (SoundManager)

	core::ecs::EntityId					player_id_ = 0;	///< プレイヤー（ECS だけの物体。ObjectFactory が持つ。以前は Player*）
	GameObjectRef						goal_;		///< ゴール（ECS だけの物体。ObjectFactory が持つ）

	// 所有リソース
	std::unique_ptr< Camera >			camera_;
	core::setting::SettingStore::Subscription	fov_subscription_;	///< 設定の FOV が変わったらカメラに反映する

	bool								is_cleared_ = false;	///< ステージクリアフラグ
	bool								bgm_loop_ = true;		///< BGM を繰り返すか（ステージの bgm.loop。以前はステージの設定 stage_config_ から読んでいた）
	float_t								action_bgm_after_timer_;

	float_t								bpm_;

	stage::StageLook					look_;					///< ステージの見た目（光の位置・色・線の揺れ。ステージの set・fade が変える）

	BalloonSoundType					balloon_sound_type_;

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
	std::optional< GameObjectRef > create_object_at_player_front( const char_t* );

	void restart();

	/// プレイヤーの行動の System・部品・位置の窓口（部品はプレイヤーがいなければ空の部品）
	ecs::PlayerBehaviorSystem* get_player_system() const;
	ecs::PlayerBehaviorComponent& get_player() const;
	ecs::PlayerInventoryComponent& get_player_inventory() const;
	ecs::EntityObject get_player_object() const;
	GameObjectRef get_player_ref() const;

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
	
	/// ステージの動きを 1 つ行う（時間を置く動きは delayed_command_list_ に預ける）
	void run_action( const stage::Action& );

	// フレームの定数の更新と描画の手順（GamePlayScene_render.cpp）。ほかのシーンでも同じ描画が要るようになったら GraphicsManager に移す
	void update_render_data_for_frame() const;
	void update_render_data_for_frame_drawing() const;
	void update_render_data_for_frame_for_eye( int ) const;

	void update_frame_constant_buffer_data_sub( FrameConstantBufferData& ) const;

	void render_to_oculus_vr() const;
	void render_to_display() const;

	void render_for_eye( float_t ortho_offset = 0.f ) const;

	void render_text() const;

	void render_far_billboards() const;
	
	void render_sprite( float_t rotho_offset = 0.f ) const;

	void render_debug_shadow_map_window() const;

public:
	explicit GamePlayScene( GameMain& game_main );
	~GamePlayScene();

	void update() override;				///< メインループ
	void render() override;				///< 描画

	bool is_clip_cursor_required() override { return true; }
	EscapeBehavior get_escape_behavior() const override { return EscapeBehavior::PAUSE_MENU; }

}; // class GamePlayScene

} // namespace blue_sky
