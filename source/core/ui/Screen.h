#pragma once

#include <core/ui/Widget.h>

#include <functional>
#include <memory>
#include <vector>

namespace core::ui
{
	class Renderer;
	class StringTable;
	struct Theme;

	/// 画面が出す効果音のきっかけ（実際の再生は呼び出し側）
	enum class UISoundEvent
	{
		MOVE,		///< フォーカス移動
		CONFIRM,	///< 決定
		CANCEL,		///< キャンセル・戻る
		CHANGE		///< 値の変更
	};

	/// ダイアログの定義
	struct DialogSpec
	{
		struct ButtonSpec
		{
			string_t text_key;
			std::function< void() > on_press;
		};

		string_t message;									///< 表示する文字列（翻訳済み or キー）
		std::function< string_t() > message_provider;		///< 毎フレーム文字列を作る（残り秒数の表示等。任意）
		std::vector< ButtonSpec > buttons;
		int cancel_index = -1;								///< キャンセル・時間切れで押したことにするボタン（-1 = キャンセル不可）
		int default_index = 0;								///< 最初にフォーカスするボタン
		float_t timeout_sec = 0.f;							///< 0 = 時間切れなし
	};

	/**
	 * UI 画面
	 *
	 * ルート層と、その上に積まれるモーダル層（ダイアログ等）を持つ。入力は最前面の層だけが受け取る。
	 *
	 * 1 フレームの処理順（update）:
	 *  1. 入力抑止中なら入力を捨てる
	 *  2. キー待ち中なら キャンセル（中止）とキー入力だけを処理
	 *  3. ポインタ（ホバー・押下・ドラッグ・離す）
	 *  4. ホイール  5. タブ  6. 方向（部品 → 階層ナビゲーション）  7. 決定  8. キャンセル
	 *  9. 部品の更新  10. post() されたコマンドの実行  11. フォーカスの修復・レイアウト
	 *
	 * 層やツリーの変更は post() 経由で行う（コールバック中にツリーが壊れないように）。
	 */
	class Screen
	{
	public:
		using SoundHandler = std::function< void( UISoundEvent ) >;

	private:
		struct Layer
		{
			std::unique_ptr< Widget > root;
			std::function< void() > on_cancel;
			Widget* focused = nullptr;
			bool dim = false;	///< 背面を暗くする
		};

		const Theme& theme_;
		const StringTable& strings_;
		const Renderer& renderer_;

		std::vector< Layer > layers_;
		std::vector< std::function< void() > > commands_;

		int suppress_frames_ = 0;
		Widget* key_capture_ = nullptr;
		bool key_capture_started_ = false;	///< キー待ちを始めたフレームか（そのフレームのキーは無視）
		Widget* pointer_capture_ = nullptr;
		bool hover_enabled_ = true;			///< ホイールで内容が動いたら、次にポインタが動くまでホバーしない
		bool layout_dirty_ = true;
		float_t last_pointer_x_ = -1.f;
		float_t last_pointer_y_ = -1.f;

		InputDevice last_device_ = InputDevice::KEYBOARD_MOUSE;
		SoundHandler sound_handler_;

		UIContext make_context();
		UIContext make_context() const;

		Layer* top_layer() { return layers_.empty() ? nullptr : & layers_.back(); }
		const Layer* top_layer() const { return layers_.empty() ? nullptr : & layers_.back(); }

		Widget* hit_test( Widget* w, float_t x, float_t y, const Rect* clip = nullptr ) const;
		Widget* find_nav_target( Widget* from, NavDir dir ) const;
		Widget* find_entry( Widget* w, NavDir dir, const Widget* from ) const;
		Widget* find_first_focusable( Widget* w ) const;
		void repair_focus();
		void run_commands();
		void layout();
		void focus_initial( Layer& );

		void handle_pointer( const UIInput&, UIContext& );
		void handle_nav( NavDir, UIContext& );
		void handle_cancel( UIContext& );

	public:
		Screen( const Theme&, const StringTable&, const Renderer& );
		~Screen();

		// ---- 層 ----

		/// ルート層を設定する（初期化時に使う。即時）。on_cancel はルートでキャンセルされたとき（戻る）
		Widget* set_root( std::unique_ptr< Widget > root, std::function< void() > on_cancel );

		/// モーダル層を積む（post される）
		void push_layer( std::unique_ptr< Widget > root, std::function< void() > on_cancel, bool dim = true );

		/// 指定した層を閉じる（post される。既に無ければ何もしない）
		void close_layer( const Widget* root );

		/// ダイアログを開く（post される）
		void show_dialog( DialogSpec spec );

		size_t get_layer_count() const { return layers_.size(); }
		Widget* get_root() const { return layers_.empty() ? nullptr : layers_.front().root.get(); }

		/// コールバック中に層・ツリーを変更するためのコマンドを積む。update の最後に実行される
		void post( std::function< void() > command );

		// ---- フレーム ----

		void update( const UIInput& input, float_t dt );
		void render( Renderer& renderer );

		/// 次の n フレームの入力を捨てる（開いた直後・最前面に戻った直後）
		void suppress_input( int frames ) { suppress_frames_ = frames; }

		void invalidate_layout() { layout_dirty_ = true; }

		// ---- フォーカス ----

		void set_focus( Widget* w, FocusReason reason );
		Widget* get_focused() const;

		/// コンテナの中の入口（最後にフォーカスした子 → 先頭）へフォーカスする（タブ切替後など）
		void focus_into( Widget* container, FocusReason reason = FocusReason::PROGRAM );

		// ---- キー割り当て待ち ----

		void begin_key_capture( Widget* w );
		void end_key_capture();
		bool is_capturing_key() const { return key_capture_ != nullptr; }

		// ---- その他 ----

		void set_sound_handler( SoundHandler h ) { sound_handler_ = std::move( h ); }
		void play( UISoundEvent e ) const { if ( sound_handler_ ) sound_handler_( e ); }

		InputDevice get_last_device() const { return last_device_; }

		/// フォーカス中の部品の説明文（翻訳済み。無ければ空）
		string_t get_description() const;

		/// 操作ヒントの 1 行（最後に使ったデバイスの表記）
		string_t build_hint_text() const;

		const Theme& get_theme() const { return theme_; }
		const StringTable& get_strings() const { return strings_; }

	}; // class Screen

} // namespace core::ui
