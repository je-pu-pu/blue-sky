#pragma once

#include <core/ui/Widget.h>

#include <functional>
#include <memory>
#include <unordered_map>
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

	/**
	 * UI 画面
	 *
	 * ルート層と、その上に積まれるモーダル層（ダイアログ等）を持つ。入力は最前面の層だけが受け取る。
	 *
	 * 1 フレームの処理順（update）:
	 *  1. 入力抑止中なら入力を捨てる
	 *  2. キー待ち中なら キャンセル（中止）とキー入力だけを処理
	 *  3. ポインタ（ホバー・押下・ドラッグ・離す）
	 *  4. ホイール  5. 文字（文字を受け取る部品がフォーカス中なら）
 *  6. アクション（タブ → 方向 → 決定 → キャンセル → 独自の操作）: フォーカス中の部品から親へ渡し、
 *     誰も使わなければ既定の処理（方向 = 階層ナビゲーション、キャンセル = 層の on_cancel、タブ = 層の中のタブ）
	 *  9. 部品の更新  10. post() されたコマンドの実行  11. フォーカスの修復・レイアウト
	 *
	 * 層やツリーの変更は post() 経由で行う（コールバック中にツリーが壊れないように）。
	 */
	class Screen
	{
	public:
		using SoundHandler = std::function< void( UISoundEvent ) >;

		/// フォーカスが移ったときに呼ばれる（読み上げ・効果などの差し込み口）
		using FocusListener = std::function< void( Widget* focused, FocusReason reason ) >;

	private:
		struct Layer
		{
			std::unique_ptr< Widget > root;
			std::function< void() > on_cancel;
			Widget* focused = nullptr;
			bool dim = false;	///< 背面を暗くする
			bool modal = true;	///< 入力を受け取る（false なら表示だけ。HUD・トーストなど、下の層の操作を妨げない）
		};

		const Theme& theme_;
		const StringTable& strings_;
		const Renderer& renderer_;

		std::vector< Layer > layers_;
		std::vector< std::function< void() > > commands_;

		std::unordered_map< ActionId, string_t > action_names_;	///< 独自の操作の、操作ヒントの名前

		int suppress_frames_ = 0;
		Widget* key_capture_ = nullptr;
		bool key_capture_started_ = false;	///< キー待ちを始めたフレームか（そのフレームのキーは無視）
		Widget* pointer_capture_ = nullptr;
		bool hover_enabled_ = true;			///< ホイールで内容が動いたら、次にポインタが動くまでホバーしない
		bool layout_dirty_ = true;
		bool reveal_focus_on_layout_ = false;	///< 次の配置で、フォーカス中の部品が見えるようにする
		float_t last_pointer_x_ = -1.f;
		float_t last_pointer_y_ = -1.f;

		InputDevice last_device_ = InputDevice::KEYBOARD_MOUSE;
		SoundHandler sound_handler_;
		FocusListener focus_listener_;

		UIContext make_context();
		UIContext make_context() const;

		/// 入力を受け取る層（上から見て最初のモーダルな層。表示だけの層は飛ばす）
		Layer* top_layer()
		{
			for ( auto i = layers_.rbegin(); i != layers_.rend(); ++i )
			{
				if ( i->modal ) return & *i;
			}
			return layers_.empty() ? nullptr : & layers_.front();
		}

		const Layer* top_layer() const { return const_cast< Screen* >( this )->top_layer(); }

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

		/// アクションをフォーカス中の部品から親へ渡し、誰も使わなければ既定の処理をする。処理したら true
		bool dispatch_action( ActionId action, UIContext& ctx );

		/// ポインタ・キーの捕捉を解除し、捕捉していた部品に知らせる
		void release_captures();

	public:
		Screen( const Theme&, const StringTable&, const Renderer& );
		~Screen();

		// ---- 層 ----

		/// ルート層を設定する（初期化時に使う。即時）。on_cancel はルートでキャンセルされたとき（戻る）
		Widget* set_root( std::unique_ptr< Widget > root, std::function< void() > on_cancel );

		/**
		 * 層を積む（post される）
		 *
		 * @param dim 背面を暗くする
		 * @param modal 入力を受け取る。false なら表示だけの層（HUD・トースト・ツールチップ）で、
		 *              入力は下のモーダルな層が受け取り続ける（以前は層はすべて入力を奪うモーダルだった）
		 */
		void push_layer( std::unique_ptr< Widget > root, std::function< void() > on_cancel, bool dim = true, bool modal = true );

		/// 指定した層を閉じる（post される。既に無ければ何もしない）
		void close_layer( const Widget* root );

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

		/**
		 * 部品を置く範囲（仮想画面からテーマのセーフエリアを除いた矩形）
		 *
		 * 各層のルートはこの範囲に置かれる。背面を暗くする塗りは画面全体に掛かる。
		 */
		Rect get_safe_rect() const;

		/// 見た目（テーマ）が変わったので配置し直す。フォーカス中の部品が見えるようにスクロールもする
		void invalidate_appearance() { layout_dirty_ = true; reveal_focus_on_layout_ = true; }

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

		/// 独自の操作（ui_action::USER 以上）の操作ヒントの名前を登録する（hint.key.<名前> / hint.pad.<名前> を引く）
		void register_action( ActionId action, const string_t& hint_name );

		void set_sound_handler( SoundHandler h ) { sound_handler_ = std::move( h ); }

		/// フォーカスが移ったときに呼ぶ関数を設定する
		void set_focus_listener( FocusListener l ) { focus_listener_ = std::move( l ); }
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
