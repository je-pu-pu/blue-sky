#pragma once

#include <blue_sky/type.h>
#include <Scene/SceneRequest.h>
#include <core/graphics/Model.h>
#include <core/graphics/Texture.h>
#include <functional>
#include <set>
#include <string>

class App;
class OculusRift;

namespace game
{
	class Config;
	class MainLoop;
}

namespace core
{
	class TimeManager;
	class Sound;

	/// 音を指すハンドル（core/sound/SoundManager.h と同じ型。シーンのヘッダが SoundManager を含まずに持てるように、ここでも宣言する）
	using SoundHandle = core::ResourceHandle< Sound >;

	namespace sound { class SoundManager; }
	namespace physics { class PhysicsManager; }

	namespace ecs
	{
		class EntityManager;
	}

	namespace setting { class SettingStore; }

	namespace graphics
	{
		namespace direct_3d_11 { class BulletDebugDraw; }
		class Texture;
	}
}

namespace blue_sky
{
	class GameMain;
	class ObjectUpdater;
	class ObjectFactory;
	class ScriptManager;
	class Input;

	namespace graphics
	{
		class GraphicsManager;
		class Model;
		class Mesh;
	}

/**
 * 場面ごとの処理、描画を管理する
 *
 */
class Scene
{
public:
	using Model				= graphics::Model;
	using Mesh				= graphics::Mesh;
	using Texture			= core::graphics::Texture;

	using Sound				= core::Sound;

	using TextureHandle		= core::graphics::TextureHandle;
	using ModelHandle		= core::graphics::ModelHandle;
	using SoundHandle		= core::SoundHandle;
	using Config			= game::Config;
	using MainLoop			= game::MainLoop;

	using EntityManager		= core::ecs::EntityManager;
	using GraphicsManager	= graphics::GraphicsManager;
	using PhysicsManager	= core::physics::PhysicsManager;
	using SoundManager		= core::sound::SoundManager;

private:
	GameMain*		game_main_;		///< 組み立ての根（作ったものが渡す。テスト用のコンストラクタでは nullptr）
	float_t total_elapsed_time_;		///< シーンが開始してからの総経過時間 ( 秒 )
	bool is_paused_;					///< ポーズ中かどうか

	std::string name_;
	std::string next_scene_;
	std::string next_stage_name_;
	SceneArguments arguments_;		///< 遷移の要求で渡された引数（作られたときに GameMain から受け取る）

	mutable std::set< std::string > lost_resource_logged_;	///< 引けなかったことをログに出した名前（毎フレーム出さない）

	/// ハンドルから引けなかったことを、名前ごとに 1 回だけログに出す
	void log_lost_resource( const char_t* what ) const;

protected:
	void update_total_elapsed_time();
	float_t get_elapsed_time() const;

	EntityManager* get_entity_manager() const;

	ObjectUpdater* get_object_updater() const;

	/// 物体を種類の名前から作り、名前で引く（ECS だけの物体を含む。物体の片付け・やり直しの入口）
	ObjectFactory* get_object_factory() const;

	core::physics::PhysicsManager* get_physics_manager() const;	///< 物理の世界（本編も ECS のテストシーンも。以前本編は get_active_object_physics の別の世界だった）
	const core::TimeManager* get_time_manager() const;			///< 時間の管理（ECS のシステムに渡す）
	GraphicsManager* get_graphics_manager() const;
	SoundManager* get_sound_manager() const;
	ScriptManager* get_script_manager() const;

	Input* get_input() const;
	Config* get_config() const;
	Config* get_save_data() const;

	OculusRift* get_oculus_rift() const;

	const MainLoop* get_main_loop() const;

	const App* get_app() const;
	const GameMain* get_game_main() const;

	/// ゲームの設定（シーンは GameMain を直接呼ばず、これらの近道を使う。設計上の問題 B28）
	core::setting::SettingStore* get_setting_store() const;

	/// 物理のデバッグ表示
	core::graphics::direct_3d_11::BulletDebugDraw* get_bullet_debug_draw() const;

	int get_width() const;
	int get_height() const;

	void play_sound( const char*, bool = false, bool = true ) const;
	void stop_sound( const char* ) const;

	void update_constant_buffer_for_sprite_frame( int line_type, float_t drawing_accent_scale = 1.f );
	
	void bind_all_render_data() const;
	void bind_game_render_data() const;
	void bind_frame_render_data() const;
	void bind_shared_object_render_data() const;

	void render_technique( const char_t*, std::function< void() > ) const;
	void render_fader() const;

	virtual Sound* get_bgm() { return 0; };

	/**
	 * ハンドルから引く（リソースは生ポインタで持ち続けず、ハンドルで持って使うたびに引く）
	 *
	 * 空・解放済みなら nullptr を返す（使う側は描かない・鳴らさない）。空でないのに引けなければ、理由を what ごとに 1 回だけログに出す
	 */
	Texture* resolve( TextureHandle, const char_t* what ) const;
	Model* resolve( ModelHandle, const char_t* what ) const;
	Sound* resolve( SoundHandle, const char_t* what ) const;

	/// 画像を名前付きで読み、そのハンドルを返す（読めなければ load_named_texture と同じく例外）
	TextureHandle load_named_texture_handle( const char_t* name, const char_t* file_name ) const;

	/// モデルを読み、そのハンドルを返す（読めなければ空のハンドル）
	ModelHandle load_model_handle( const char_t* name ) const;

	/// ハンドルの音を鳴らす（引けなければ鳴らさない）
	void play( SoundHandle, const char_t* what, bool loop = false ) const;

	/// テスト用 : GameMain なしで作る（シーンの切り替えを偽のシーンで確かめるため。近道・update() は使えない）
	explicit Scene( const SceneArguments& arguments );

public:
	/// 組み立ての根（GameMain）を受け取る。シーンは GameMain のシングルトンを呼ばない（設計上の問題 B28）
	explicit Scene( GameMain& game_main );
	virtual ~Scene();

	void set_name( const std::string& name ) { name_ = name; }
	const std::string& get_name() const { return name_; }

	virtual void update();					///< メインループ
	virtual void render() = 0;				///< 描画

	virtual void on_function_key_down( int ) { };

	/// オーバーレイシーン ( ポーズメニューなど ) かどうか
	virtual bool is_overlay() const { return false; }

	/// キー割り当てで、押されたキーを待っているか（待っている間は、ファンクションキーの切り替えを動かさない）
	virtual bool is_capturing_key() const { return false; }

	/// ESC を押したときにすること（オーバーレイが開いていないとき）
	enum class EscapeBehavior
	{
		DEFAULT,		///< 既定（全ステージをクリア済みならタイトルへ、そうでなければ終了）
		PAUSE_MENU,		///< ポーズメニューを開く（ゲームプレイ中）
		QUIT,			///< 終了する（タイトル）
		NONE,			///< 何もしない
	};

	/// ESC を押したときにすることを申告する（以前は GameMain がシーン名で分岐していた）
	virtual EscapeBehavior get_escape_behavior() const { return EscapeBehavior::DEFAULT; }

	/// オーバーレイシーンの場合、下のシーンを描画するかどうか（false なら、これより下はベースシーンも含めて描かない）
	virtual bool is_transparent() const { return false; }

	/// オーバーレイシーンの場合、下に積まれたオーバーレイを隠すかどうか（ベースシーンは is_transparent() に従う）
	/// メニューの上にメニューを重ねるとき（ポーズ → オプション）に、下のメニューが透けたり暗幕が二重になったりしないようにする
	virtual bool hides_overlays_below() const { return false; }

	/// オーバーレイシーンが再び最前面に戻ったとき（上に積んだオーバーレイが閉じたとき）に呼ばれる
	virtual void on_resume() { }

	/// ポーズ状態にする
	void set_paused( bool paused ) { is_paused_ = paused; }
	bool is_paused() const { return is_paused_; }

	bool is_first_game_play() const;
	unsigned int get_current_time() const;

	const std::string& get_next_scene() const { return next_scene_; }
	void set_next_scene( const std::string& next_scene ) { next_scene_ = next_scene; }

	const std::string& get_stage_name() const;

	/// 遷移の要求で渡された引数（"push:options?tab=controls" の tab）。無ければ default_value。コンストラクタの中でも読める
	string_t get_argument( const string_t& key, const string_t& default_value = string_t() ) const
	{
		const auto i = arguments_.find( key );
		return i != arguments_.end() ? i->second : default_value;
	}
	
	const std::string& get_next_stage_name() const { return next_stage_name_; }
	void set_next_stage_name( const std::string& next_stage_name ) { next_stage_name_ = next_stage_name; }

	virtual bool is_clip_cursor_required() { return false; }

	float_t get_total_elapsed_time() const { return total_elapsed_time_; }
	void reset_total_elapsed_time() { total_elapsed_time_ = 0.f; }

}; // class Scene

} // namespace blue_sky
