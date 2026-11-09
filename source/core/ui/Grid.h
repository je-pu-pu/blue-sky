#pragma once

#include <core/ui/Widget.h>

namespace core::ui
{

/**
 * 格子状に並べるコンテナ（インベントリ・セーブスロット・ソフトキーボードなど）
 *
 * 子を左上から右へ、columns 個ごとに次の行へ並べる。セルの幅は等分、行の高さはその行で一番高い子に合わせる。
 * 方向キーで上下左右の隣のセルへ移る（Stack は 1 方向にしか移れなかった。設計上の問題 B8）。
 * 端では祖先のコンテナへ抜ける。
 */
class Grid : public Widget
{
private:
	int columns_;
	float_t spacing_ = -1.f;		///< セルの間隔（負ならテーマの spacing）

	std::vector< float_t > row_heights_;

	float_t get_spacing( const UIContext& ctx ) const;

public:
	explicit Grid( int columns ) : columns_( columns > 0 ? columns : 1 ) { }

	void set_spacing( float_t s ) { spacing_ = s; }
	int get_columns() const { return columns_; }

	float_t measure_height( float_t width, const UIContext& ctx ) const override;
	void arrange( const Rect& rect, const UIContext& ctx ) override;

	int nav_next_index( int index, NavDir dir ) const override;

}; // class Grid

} // namespace core::ui
