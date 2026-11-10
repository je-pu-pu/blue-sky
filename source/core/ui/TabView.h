#pragma once

#include <core/ui/Widget.h>
#include <core/ui/ClickTracker.h>

namespace core::ui
{

/**
 * タブ（上に見出し行、下に現在のページ）
 *
 * - Q/E・PageUp/PageDown・LB/RB（UIInput の tab_prev / tab_next）と、見出しのクリックで切り替える
 * - 子として公開するのは現在のページだけ。非表示のページはナビゲーション・ヒット判定・描画・更新の対象外
 * - ページごとに最後のフォーカス位置を覚えていて、戻ったときにそこへフォーカスする
 */
class TabView : public Widget
{
private:
	std::vector< string_t > titles_;
	std::vector< bool > badges_;		///< 見出しに印（新しい項目がある・未適用の変更がある など）を付けるか
	size_t current_ = 0;
	std::vector< Rect > tab_rects_;
	ClickTracker click_;

public:
	/// 捕捉が外れた（層が積まれたなど）: 押している・ドラッグ中の状態を解除する
	void on_capture_lost() override { click_.cancel(); }

	/// ページを追加する（ページの型と引数を指定。型付きポインタを返す）
	template< typename T, typename ... Args >
	T* add_page( const string_t& title_key, Args&& ... args )
	{
		titles_.push_back( title_key );
		badges_.push_back( false );
		return add< T >( std::forward< Args >( args ) ... );
	}

	/**
	 * 見出しに印を付ける・外す（以前は見出しが矩形の配列で、印を付けられなかった。設計上の問題 B9）
	 */
	void set_badge( size_t index, bool on ) { if ( index < badges_.size() ) badges_[ index ] = on; }
	bool has_badge( size_t index ) const { return index < badges_.size() && badges_[ index ]; }

	/// 見出しの矩形（配置の後）
	const Rect* get_tab_rect( size_t index ) const { return index < tab_rects_.size() ? & tab_rects_[ index ] : nullptr; }

	size_t get_page_count() const { return children_.size(); }
	size_t get_current() const { return current_; }
	Widget* get_page( size_t i ) const { return i < children_.size() ? children_[ i ].get() : nullptr; }

	/// ページを切り替える（フォーカスはそのページの入口へ移る）
	void select( size_t index, UIContext& ctx );

	size_t child_count() const override { return children_.empty() ? 0 : 1; }
	Widget* child_at( size_t ) const override { return children_[ current_ ].get(); }

	bool accepts_pointer() const override { return true; }
	bool has_tabs() const override { return children_.size() > 1; }

	float_t measure_height( float_t width, const UIContext& ctx ) const override;
	void arrange( const Rect& rect, const UIContext& ctx ) override;

	bool on_tab( int direction, UIContext& ctx ) override;
	bool on_pointer( const PointerEvent& e, UIContext& ctx ) override;

	void render( Renderer& renderer, const UIContext& ctx ) const override;

}; // class TabView

} // namespace core::ui
