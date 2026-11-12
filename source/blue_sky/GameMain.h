#pragma once


#include "App.h"
#include "TimeManager.h"

#include <game/Game.h>

#include <type/type.h>

#include <Scene/SceneRequest.h>
#include <Scene/InputRouting.h>
#include <blue_sky/dev/DevOptions.h>
#include <blue_sky/SceneStack.h>

#include <deque>
#include <unordered_map>
#include <vector>
#include <blue_sky/DebugLog.h>
#include <GameObject/GameObjectContext.h>

namespace core::ecs { class EntityManager; }

namespace blue_sky::dev
{
	class DevTools;
	class DebugConsole;
}

class DirectWrite;

class DirectInput;

class OculusRift;

namespace game
{
	class Config;
	class MainLoop;
};

namespace core
{
	namespace sound
	{
		class SoundEngine;
		class SoundManager;
	}

	namespace physics
	{
		class PhysicsManager;
	}

	namespace graphics
	{
		class DebugDraw;
	}

	namespace graphics::direct_3d_11
	{
		class Direct3D11;
		class BulletDebugDraw;
	}

	namespace setting
	{
		class SettingStore;
	}

	namespace ui
	{
		class StringTable;
		struct Theme;
	}
}

namespace blue_sky
{
	class Input;

	class ObjectUpdater;
	class ObjectFactory;

	class SceneManager;
	class ScriptManager;

	class Camera;

	class Scene;
	class Fader;

	namespace graphics
	{
		class GraphicsManager;
	}

	using graphics::GraphicsManager;
};

namespace blue_sky
{

class GameMain : public game::Game
{
public:
	using Config					= game::Config;
	using MainLoop					= game::MainLoop;

	using SoundEngine				= core::sound::SoundEngine;

	using GraphicsManager			= GraphicsManager;
	using SoundManager				= core::sound::SoundManager;
	using PhysicsManager			= core::physics::PhysicsManager;

	using Direct3D					= core::graphics::direct_3d_11::Direct3D11;
	using BulletDebugDraw			= core::graphics::direct_3d_11::BulletDebugDraw;

private:
	App&									app_;					///< ウィンドウ（App が GameMain を作るときに渡す。以前は App のシングルトンから引いていた）
	const dev::DevOptions					dev_options_;			///< 開発用の環境変数（起動時に 1 回だけ読む。以前は各所でその場で読んでいた）
	TimeManager								time_manager_;			///< 時間管理

	std::unique_ptr< Direct3D >				direct_3d_;				///< Direct3D

	std::unique_ptr< DirectInput >			direct_input_;			///< DirectInput
	ScopedCoutToLog							cout_to_log_;			///< std::cout をログ（log/blue-sky.log）にも流す（ログの出口をそろえる）
	std::unique_ptr< Input >				input_;					///< Game Input

	std::unique_ptr< OculusRift >			oculus_rift_;			///< Oculus Rift

	std::unique_ptr< SoundEngine >			sound_engine_;			///< SoundEngine

	std::unique_ptr< PhysicsManager >		physics_manager_;		///< 物理の世界（本編と ECS のテストシーンで 1 つ。シーンを替えるときに clear する。以前は本編だけの ActiveObjectPhysics と 2 つあった）
	std::unique_ptr< GraphicsManager >		graphics_manager_;		///< GraphicsManager
	std::unique_ptr< SoundManager >			sound_manager_;			///< SoundManager
	std::unique_ptr< ScriptManager >		script_manager_;		///< ScriptManager

	std::unique_ptr< Config >				save_data_;				///< Save Data

	std::unique_ptr< MainLoop >				main_loop_;				///< ループ管理

	GameObjectContext						game_object_context_;	///< ゲームのオブジェクトに渡すもの（物理の世界・音・経過時間）
	std::unique_ptr< ObjectUpdater >		object_updater_;		///< 本編の物体の System を決まった順に動かす（以前の ActiveObjectManager の残り）
	std::unique_ptr< ObjectFactory >		object_factory_;		///< 物体を種類の名前から作り、名前で引く・やり直す・片付ける

	std::unique_ptr< BulletDebugDraw >		bullet_debug_draw_;		///< 物理のデバッグ表示（Direct3D 11 の実装。ほかへは抽象の DebugDraw として渡す）
	std::unique_ptr< Fader >				fader_;					///< 画面のフェード（ゲームの演出。シーンは Scene::get_fader・render_fader で使う。以前は描画の窓口が持っていた）

	std::unique_ptr< SceneManager >			scene_manager_;			///< シーンの作り方の一覧（以前はシングルトン）
	std::unique_ptr< core::ecs::EntityManager >	entity_manager_;	///< ECS の Entity・Component・System（以前はシングルトン。シーンより後に壊す）

	bool									is_display_fps_;		///< FPS 表示フラグ

	bool									is_show_cursor_;		///< 現在カーソルを表示するかどうか

	std::unique_ptr< core::setting::SettingStore >	settings_;		///< ゲーム設定（オプション画面が編集する）
	std::unique_ptr< core::ui::StringTable >		ui_strings_;	///< UI の表示文字列
	std::unique_ptr< core::ui::Theme >				ui_theme_;		///< UI の見た目（文字の大きさの倍率を掛けたもの）
	std::unique_ptr< core::ui::Theme >				ui_theme_base_;	///< 倍率を掛ける前の見た目（既定 ＋ テーマファイル）
	float_t											ui_text_scale_ = 1.f;	///< 文字の大きさの倍率（設定 ui.text_scale）
	bool											is_display_change_requested_ = false;	///< 解像度・フルスクリーンの変更をフレーム末に行う
	bool											is_full_screen_toggle_requested_ = false;	///< ユーザーによるフルスクリーンの切り替え（Alt+Enter / F5）をフレーム末に行う
	bool											is_shut_down_ = false;					///< shutdown() 済み
	/// 開発用の機能（フレームの保存・フレームの時間・シーンを順に替える・解像度の変更の試し・経過時間の固定）と、imgui の開発用の窓（F1）。
	/// どちらも外には見せない。scene_stack_ の次に壊れる（使う GameMain の持ち物より先）
	std::unique_ptr< dev::DevTools >				dev_tools_;
	std::unique_ptr< dev::DebugConsole >			debug_console_;

	/// 開発用の機能を作る（保存・設定の変更・シーンの切り替えの仕事を渡す）
	void create_dev_tools();

	/// ベースのシーンとオーバーレイ・切り替えの要求（外には見せない）。
	/// シーンは設定（購読）・文字列表・テーマ・ECS など GameMain の持ち物を使うので、最後に宣言して最初に壊す（デストラクタでも最初に壊す）
	std::unique_ptr< SceneStack >					scene_stack_;

	void flush_resource_releases();

	void apply_display_settings();

	void update_render_data_for_game() const;

	/// ESC を押したときの処理（オーバーレイが開いていないとき。何をするかは SceneStack が決める）
	void handle_escape();

	/// ベースを壊した後、次のシーンを作る前の片付け（物体・ECS・物理の世界・解放を頼まれたリソース）
	void clear_for_next_scene();

	/// シーンの持ち物（SceneStack）を作る。シーンを作る・片付ける・画面に出す仕事を渡す
	void create_scene_stack();

public:
	explicit GameMain( App& app );
	virtual ~GameMain() override;

	void setup_scene();

	bool update() override;

	void render();

	void on_key_down( char_t ) override;

	/// キーやマウスのボタンが押された（短いタップを取りこぼさないよう Input に溜める）
	void on_raw_key_down( int vk ) override;
	void on_raw_mouse_move( long dx, long dy ) override;

	/// IME で変換中の文字が変わった（確定・取り消しで空）
	void on_ime_composition( const std::u32string& text ) override;
	void on_special_key_down( int ) override;
	void on_mouse_wheel( int ) override;
	void on_resize() override;
	void on_toggle_full_screen() override { is_full_screen_toggle_requested_ = true; }
	void shutdown() override;
	unsigned long get_idle_wait_msec() const override;

	/// ユーザーの設定（settings.config）を保存する
	void save_settings();

	Input* get_input() const { return input_.get(); }
	
	OculusRift* get_oculus_rift() const { return oculus_rift_.get(); }

	PhysicsManager* get_physics_manager() const { return physics_manager_.get(); }	///< 物理の世界（本編も ECS のテストシーンも。以前は core::Service から取っていた）
	const TimeManager* get_time_manager() const { return & time_manager_; }		///< 時間の管理（以前は core::Service から取っていた）
	GraphicsManager* get_graphics_manager() const { return graphics_manager_.get(); }
	SoundManager* get_sound_manager() const { return sound_manager_.get(); }
	ScriptManager* get_script_manager() const { return script_manager_.get(); }

	ObjectUpdater* get_object_updater() const { return object_updater_.get(); }
	ObjectFactory* get_object_factory() const { return object_factory_.get(); }
	
	App* get_app() const { return & app_; }

	/// ECS の Entity・Component・System
	core::ecs::EntityManager* get_entity_manager() const { return entity_manager_.get(); }

	inline int get_width() const override { return get_app()->get_width(); }
	inline int get_height() const override { return get_app()->get_height(); }

	Config* get_config() const { return get_app()->get_config(); }
	Config* get_save_data() const { return save_data_.get(); }

	const MainLoop* get_main_loop() const { return main_loop_.get(); }

	/// デバッグ表示の線（物理のデバッグ表示と同じ所に描く）
	core::graphics::DebugDraw* get_debug_draw() const;

	/// 画面のフェード
	Fader* get_fader() const { return fader_.get(); }

	const std::string& get_stage_name() const { return scene_stack_->get_stage_name(); }
	void set_stage_name( const std::string& stage_name ) { scene_stack_->set_stage_name( stage_name ); }

	bool is_display_fps() const { return is_display_fps_; }
	/// 垂直同期を待って表示するか
	void set_vsync( bool v );

	void set_display_fps( bool f ) { is_display_fps_ = f; }

	core::setting::SettingStore* get_settings() const { return settings_.get(); }
	const core::ui::StringTable& get_ui_strings() const { return *ui_strings_; }
	const core::ui::Theme& get_ui_theme() const { return *ui_theme_; }

	/// 解像度・フルスクリーンの変更を予約する（UI のコールバック中にウィンドウを変えないよう、update の最後に行う）
	void request_display_change() { is_display_change_requested_ = true; }

	/// 文字の大きさの倍率を UI のテーマに反映する（テーマファイルで決めた大きさに掛ける）
	void set_ui_text_scale( float_t scale );

	const Scene* get_current_scene() const { return scene_stack_->get_base_scene(); }

	/// シーン遷移を要求する（実行はフレームの境目）
	void request_scene( const SceneRequest& request ) { scene_stack_->request( request ); }

	/// 今作っているシーンに渡す引数（Scene のコンストラクタが受け取る。作っている間だけ中身がある）
	const SceneArguments& get_next_scene_arguments() const { return scene_stack_->get_next_scene_arguments(); }

	/// オーバーレイシーンが存在するか
	bool has_overlay_scene() const { return scene_stack_->has_overlay(); }

	void set_show_cursor( bool show_cursor ) { is_show_cursor_ = show_cursor; get_app()->show_cursor( is_show_cursor_ ); }

	/// カーソルの表示と閉じ込めを、今の状態（コマンドモード・メニュー・シーンの要求）から決める
	void update_cursor_state();
	bool is_show_cursor() const override { return is_show_cursor_; }

	/// ゲームがマウスを取っているか（カーソルを隠して窓に閉じ込め、マウスで視点を回している間）
	bool is_mouse_captured_by_game();

	InputOwner last_input_owner_ = InputOwner::BASE_SCENE;		///< 前のフレームで入力を受け取った先（変わったときだけログに出す）
};

} // namespace blue_sky

