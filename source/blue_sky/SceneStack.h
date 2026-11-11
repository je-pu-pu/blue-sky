#pragma once

#include <blue_sky/type.h>
#include <Scene/SceneRequest.h>

#include <deque>
#include <functional>
#include <memory>
#include <vector>

namespace blue_sky
{

class Scene;

/**
 * ベースのシーンとオーバーレイを持ち、切り替えの要求を集めてフレームの区切りで実行する
 *
 * 以前は GameMain の中にあった（設計上の問題 D2。doc/GameMainの分割.md）。GameMain の持ち物で、外には見せない。
 * シーンを作る・片付ける・画面に出すなどの GameMain の仕事は Hooks で受け取る（テストでは偽物を渡す）。
 *
 * - 積む・外す・置き換える要求は request() でキューに積み、process_requests() でまとめて実行する
 * - ベースの置き換えは、次のフレームに「読み込み中」を出してから行う（is_loading_frame()・replace_base_with_loading_screen()）
 * - 未登録のシーンへの切り替えは、ログに出して無視する
 */
class SceneStack
{
public:
	/// シーンを作る（名前と引数 → シーン。作れなければ nullptr か例外 common::exception< std::string >）
	using SceneFactory = std::function< std::unique_ptr< Scene >( const string_t& name, const SceneArguments& args ) >;

	/// GameMain から受け取る仕事（設定しないものは何もしない。is_registered・create_scene は要る）
	struct Hooks
	{
		std::function< bool( const string_t& ) >	is_registered;				///< 登録済みのシーンか
		SceneFactory								create_scene;				///< シーンを作る

		std::function< void( const string_t& ) >	before_base_destroyed;		///< ベースを壊す前（音のグループを替える）
		std::function< void() >						after_base_destroyed;		///< ベースを壊した後、次を作る前（物体・ECS・物理の世界を片付け、リソースを解放する）
		std::function< void() >						show_loading_screen;		///< 「読み込み中」を表示する
		std::function< void() >						after_base_replaced;		///< 読み込み中の画面からベースを置き換えた後（セーブデータを保存する）

		std::function< void() >						update_cursor;				///< カーソルの表示と閉じ込めを決め直す
		std::function< void() >						on_all_overlays_closed;		///< オーバーレイがすべて閉じた（入力のブロックを外し、カーソルを決め直す）

		std::function< void() >						quit;						///< アプリを終える
		std::function< void( const char_t* ) >		play_sound;					///< 効果音を鳴らす（ESC の ok・cancel）
		std::function< bool() >						can_return_to_title;		///< ESC（既定）でタイトルへ戻れるか（最初のステージをすべてクリア済み）。false なら終える
	};

	/// 起動時のオーバーレイを開くまでのフレーム数（ベースシーンを数フレーム更新してから開く。一度も更新されていないシーンは描画の準備ができていない）
	static constexpr int START_OVERLAY_WAIT_FRAMES = 10;

private:
	Hooks										hooks_;

	std::unique_ptr< Scene >					base_;						///< 現在のベースシーン
	std::vector< std::unique_ptr< Scene > >		overlays_;					///< オーバーレイのスタック（後ろが最前面）
	string_t									stage_name_;				///< 現在のステージ名

	std::deque< SceneRequest >					requests_;					///< 切り替えの要求（フレームの区切りでまとめて実行する）
	SceneRequest								pending_base_request_;		///< 次のフレームで行うベースの置き換え
	bool										is_loading_frame_ = false;	///< 次のフレームは「読み込み中」を表示してからベースを置き換える
	SceneArguments								next_scene_arguments_;		///< 今作っているシーンに渡す引数（作り終えたら空にする）

	string_t									pending_start_overlay_;		///< 起動時に開くオーバーレイ
	int											start_overlay_wait_frames_ = 0;

	std::unique_ptr< Scene > generate_scene( const string_t& name, const SceneArguments& args );

public:
	explicit SceneStack( Hooks hooks );
	~SceneStack();

	SceneStack( const SceneStack& ) = delete;
	SceneStack& operator = ( const SceneStack& ) = delete;

	/// 起動時のベースシーンを作る（未登録なら title にする）
	void start( const string_t& scene_name );

	/// 起動時のオーバーレイを予約する（START_OVERLAY_WAIT_FRAMES フレーム後に積む要求を出す。空なら開かない）
	void set_start_overlay( const string_t& name ) { pending_start_overlay_ = name; start_overlay_wait_frames_ = 0; }

	/// 毎フレーム呼ぶ。起動時のオーバーレイを開く時が来たら、積む要求を出す
	void update_start_overlay();

	/// 切り替えを要求する（実行はフレームの区切りの process_requests）
	void request( const SceneRequest& r ) { requests_.push_back( r ); }

	/// シーンが出した切り替えの文字列（Scene::set_next_scene）を要求にしてキューに積む
	void collect_requests();

	/// キューの要求を順に実行する。ベースの置き換えは次のフレームに回す（「読み込み中」を表示するため）
	void process_requests();

	/// 次のフレームで「読み込み中」を表示してからベースを置き換えるか
	bool is_loading_frame() const { return is_loading_frame_; }

	/// 「読み込み中」を表示し、ベースを置き換え、残りの要求を処理する
	void replace_base_with_loading_screen();

	/// ESC が押されたとき（escape_pushed）の処理。オーバーレイが開いている間は UI が処理するので何もしない
	void handle_escape( bool escape_pushed );

	/// ベースシーンをすぐに作り直す（未登録なら無視。作れなければ title にする。title も作れなければ例外）
	void setup_base_scene( const string_t& name, const SceneArguments& args = SceneArguments() );

	/// オーバーレイをすぐに積む（ベースはポーズする。未登録・オーバーレイ用でないシーンは積まない）
	void push_overlay( const string_t& name, const SceneArguments& args = SceneArguments() );

	/// 最前面のオーバーレイをすぐに外す（すべて閉じたらベースのポーズを解く）
	void pop_overlay();

	/// シーンをすべて壊す（オーバーレイ → ベースの順）
	void clear();

	Scene* get_base_scene() const { return base_.get(); }
	const std::vector< std::unique_ptr< Scene > >& get_overlays() const { return overlays_; }
	Scene* get_top_overlay() const { return overlays_.empty() ? nullptr : overlays_.back().get(); }
	bool has_overlay() const { return ! overlays_.empty(); }

	const string_t& get_stage_name() const { return stage_name_; }
	void set_stage_name( const string_t& stage_name ) { stage_name_ = stage_name; }

	/// 今作っているシーンに渡す引数（Scene のコンストラクタが受け取る。作っている間だけ中身がある）
	const SceneArguments& get_next_scene_arguments() const { return next_scene_arguments_; }
};

} // namespace blue_sky
