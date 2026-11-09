#pragma once

#include <core/type.h>
#include <core/ui/Rect.h>
#include <core/ui/UIInput.h>

#include <functional>
#include <memory>
#include <vector>

namespace core::ui
{
	class Renderer;
	class Screen;
	class StringTable;
	struct Theme;

	/**
	 * 部品に渡す共通の文脈
	 */
	struct UIContext
	{
		const Theme& theme;
		const StringTable& strings;
		const Renderer& renderer;	///< テキスト計測用
		Screen& screen;

		/// 文字列キーを表示文字列にする（未登録ならキーそのもの）
		const string_t& tr( const string_t& key ) const;
	};

	/// フォーカスが移った理由
	enum class FocusReason
	{
		NAV,		///< 方向キー
		POINTER,	///< マウスのホバー・クリック
		PROGRAM		///< プログラム（初期フォーカス、無効化による移動など）
	};

	/// 並べる向き（ナビゲーションの軸）
	enum class Axis
	{
		NONE,		///< 並べない（ナビゲーションは子をそのまま辿る）
		VERTICAL,
		HORIZONTAL
	};

	/// 操作ヒントの種類
	enum class HintAction
	{
		CONFIRM,	///< 決定
		CHANGE,		///< 左右で変更
		CANCEL,		///< 戻る
		TAB			///< タブ切替
	};

	struct Hint
	{
		HintAction action;
		string_t text_key;
	};

	/// ポインタ（マウス左ボタン）のイベント
	struct PointerEvent
	{
		enum class Type { PRESS, DRAG, RELEASE };

		Type type;
		float_t x;
		float_t y;
		bool inside;	///< 部品の矩形内か
	};

	/**
	 * UI 部品の基底クラス
	 *
	 * - 親が子を所有する（std::unique_ptr）。add<T>() は型付きの非所有ポインタを返す
	 * - 部品は値を持たない。値は getter/setter（設定ストア等）から毎フレーム読む
	 * - 入力はフォーカス中の部品から順に渡され、消費したら true を返す
	 * - 層やツリーを変更したいときは Screen::post() を使う（コールバック中の直接変更は禁止）
	 */
	class Widget
	{
		friend class Screen;
		friend struct WidgetAccess;

	protected:
		Rect rect_;
		Widget* parent_ = nullptr;
		std::vector< std::unique_ptr< Widget > > children_;

		bool visible_ = true;
		std::function< bool() > enabled_if_;
		bool focused_ = false;
		Widget* last_focused_child_ = nullptr;	///< 最後にフォーカスした子孫を含む子（ナビゲーションの入口）

		string_t description_key_;

		/// UIContext 経由で文字列を引くための補助
		static const string_t& tr( const UIContext& ctx, const string_t& key ) { return ctx.tr( key ); }

	public:
		virtual ~Widget();

		// ---- ツリー ----

		template< typename T, typename ... Args >
		T* add( Args&& ... args )
		{
			auto child = std::make_unique< T >( std::forward< Args >( args ) ... );
			T* raw = child.get();
			child->parent_ = this;
			children_.push_back( std::move( child ) );
			return raw;
		}

		void remove_all_children();

		Widget* parent() const { return parent_; }

		/// ナビゲーション・ヒット判定・描画の対象となる子（TabView は現在のページだけを返す）
		virtual size_t child_count() const { return children_.size(); }
		virtual Widget* child_at( size_t i ) const { return children_[ i ].get(); }

		/// 子の中での位置（見つからなければ -1）
		int index_of_child( const Widget* child ) const;

		/// 自分が w の祖先か（w 自身を含む）
		bool contains_widget( const Widget* w ) const;

		// ---- 状態 ----

		void set_visible( bool v ) { visible_ = v; }
		bool is_visible() const { return visible_; }

		void set_enabled( bool e ) { enabled_if_ = [e] { return e; }; }
		void set_enabled_if( std::function< bool() > f ) { enabled_if_ = std::move( f ); }
		bool is_enabled() const { return ! enabled_if_ || enabled_if_(); }

		/// フォーカスを受け取れる種類の部品か（Button / Slider 等が true）
		virtual bool is_focusable_type() const { return false; }

		/// 今フォーカスを受け取れるか（種類・表示・有効・祖先の表示を考慮）
		bool can_focus() const;

		/// 祖先を含めて表示中か
		bool is_visible_in_tree() const;

		bool is_focused() const { return focused_; }

		void set_description_key( const string_t& key ) { description_key_ = key; }
		const string_t& get_description_key() const { return description_key_; }

		// ---- ナビゲーション ----

		virtual Axis nav_axis() const { return Axis::NONE; }
		virtual bool nav_wrap() const { return false; }

		/**
		 * 子孫を見せる範囲（ScrollView の表示範囲）。nullptr なら制限なし
		 *
		 * 範囲がある場合、範囲に完全に収まらない子孫は描画もヒット判定もしない
		 * （コンテナは中へ降りて、収まる子だけを扱う）。テキストが途中で欠けることがない。
		 */
		virtual const Rect* get_view_clip() const { return nullptr; }

		/// 子孫の矩形が見えるようにする（ScrollView が実装）
		virtual void ensure_visible( const Rect& ) { }

		/// フォーカスは受けないがポインタのイベントは受けたいか（TabView の見出し等）
		virtual bool accepts_pointer() const { return false; }

		/// タブ切替を持つか（操作ヒントに「タブ」を出す判定）
		virtual bool has_tabs() const { return false; }

		// ---- レイアウト ----

		/// 幅を与えたときの希望の高さ
		virtual float_t measure_height( float_t width, const UIContext& ctx ) const;

		/// 矩形を確定する（コンテナは子を配置する）
		virtual void arrange( const Rect& rect, const UIContext& ctx );

		const Rect& rect() const { return rect_; }

		// ---- 入力（消費したら true） ----

		virtual bool on_nav( NavDir, UIContext& ) { return false; }
		virtual bool on_confirm( UIContext& ) { return false; }
		virtual bool on_cancel( UIContext& ) { return false; }
		virtual bool on_pointer( const PointerEvent&, UIContext& ) { return false; }
		virtual bool on_wheel( int, UIContext& ) { return false; }
		virtual bool on_tab( int /* -1 or +1 */, UIContext& ) { return false; }
		virtual bool on_key_captured( int, UIContext& ) { return false; }

		/// 毎フレームの更新（子も更新する）
		virtual void update( float_t dt, UIContext& ctx );

		// ---- 描画・ヒント ----

		/// 描画（既定は子を描画する）
		virtual void render( Renderer& renderer, const UIContext& ctx ) const;

		/// 子だけを描画する
		void render_children( Renderer& renderer, const UIContext& ctx ) const;

		/**
		 * 範囲に完全に収まるものだけを描画する（ScrollView が使う）
		 *
		 * 自分が範囲に収まれば普通に描画し、はみ出すコンテナは子へ降りて収まるものだけ描画する。
		 */
		void render_clipped( Renderer& renderer, const UIContext& ctx, const Rect& clip ) const;

		/// rect が clip に完全に収まるか
		static bool is_inside( const Rect& rect, const Rect& clip )
		{
			return rect.x >= clip.x && rect.y >= clip.y && rect.right() <= clip.right() && rect.bottom() <= clip.bottom();
		}

		/// フォーカス中に使える操作
		virtual void collect_hints( std::vector< Hint >& ) const { }

	}; // class Widget

} // namespace core::ui
