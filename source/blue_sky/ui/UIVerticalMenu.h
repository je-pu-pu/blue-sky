#pragma once

#include <blue_sky/type.h>

#include <string>
#include <vector>
#include <functional>

namespace blue_sky
{
	class Input;
}

namespace blue_sky::ui
{
	class UIRenderer;

/**
 * 縦並びメニュー
 *
 * キーボード / コントローラーで操作可能な縦並びのテキストメニュー。
 * オプション画面やポーズメニューなどのオーバーレイシーンで使用する。
 */
class UIVerticalMenu
{
public:
	struct Item
	{
		string_t text;
		std::function< void() > on_select;
	};

private:
	std::vector< Item > items_;
	int selected_index_ = 0;

	// レイアウト
	float_t x_ = 0.f;
	float_t y_ = 0.f;
	float_t width_ = 400.f;
	float_t item_height_ = 80.f;

	// カーソル
	string_t cursor_text_ = "> ";

	// 色
	Color normal_color_;
	Color selected_color_;

	// マウス操作用
	int last_mouse_x_ = -1;
	int last_mouse_y_ = -1;

public:
	UIVerticalMenu();

	/// メニュー項目の追加・更新・クリア
	void add_item( const string_t& text, std::function< void() > on_select );
	void set_item_text( int index, const string_t& text );
	void clear_items();

	/// レイアウト設定
	void set_position( float_t x, float_t y );
	void set_width( float_t w );
	void set_item_height( float_t h );
	void set_cursor_text( const string_t& text ) { cursor_text_ = text; }

	/// 色設定
	void set_colors( const Color& normal, const Color& selected );

	/// 画面中央に配置する（UIRenderer から画面サイズを取得）
	void center_on_screen( const UIRenderer& renderer );

	/// 入力処理（上下キー / 決定ボタン / マウス）
	void update( Input* input, const UIRenderer& renderer );

	/// 描画
	void render( UIRenderer& renderer );

	/// 選択状態
	int get_selected_index() const { return selected_index_; }
	void set_selected_index( int index );

	void select_next();
	void select_prev();

	int get_item_count() const { return static_cast< int >( items_.size() ); }

}; // class UIVerticalMenu

} // namespace blue_sky::ui
