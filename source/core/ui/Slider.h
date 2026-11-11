#pragma once

#include <core/ui/LabeledRow.h>

#include <functional>

namespace core::ui
{

/**
 * 数値スライダー
 *
 * 左右（長押しでリピート）で step ずつ変える。溝をクリック・ドラッグすると位置の値になる
 * （行の中で押したときだけ。Enter で値が飛ぶことは無い）。
 */
class Slider : public LabeledRow
{
public:
	/// 捕捉が外れた（層が積まれたなど）: 押している・ドラッグ中の状態を解除する
	void on_capture_lost() override { if ( dragging_ ) { dragging_ = false; commit(); } }

	using Getter = std::function< float_t() >;
	using Setter = std::function< void( float_t ) >;
	using Formatter = std::function< string_t( float_t ) >;

private:
	Getter getter_;
	Setter setter_;
	float_t min_;
	float_t max_;
	float_t step_;
	Formatter formatter_;

	bool dragging_ = false;
	std::function< void() > on_commit_;

	/// 操作を終えた（ドラッグを離した・キーで 1 段動かした）ことを知らせる
	void commit() { if ( on_commit_ ) on_commit_(); }
	Rect track_rect_;
	Rect text_rect_;

	void set_value( float_t v, UIContext& ctx );
	float_t value_from_x( float_t x ) const;

public:
	Slider( const string_t& label_key, Getter getter, Setter setter, float_t min, float_t max, float_t step )
		: LabeledRow( label_key ), getter_( std::move( getter ) ), setter_( std::move( setter ) ), min_( min ), max_( max ), step_( step ) { }

	void set_formatter( Formatter f ) { formatter_ = std::move( f ); }

	float_t get_value() const { return getter_ ? getter_() : min_; }
	AccessibleInfo describe( const UIContext& ctx ) const override;
	const Rect& get_track_rect() const { return track_rect_; }
	bool is_dragging() const { return dragging_; }

	/// 操作を終えたとき（ドラッグを離した・キーで 1 段動かした）に呼ぶ関数（設定の ON_RELEASE の反映に使う）
	void set_on_commit( std::function< void() > f ) { on_commit_ = std::move( f ); }

	void arrange( const Rect& rect, const UIContext& ctx ) override;

	bool on_nav( NavDir dir, UIContext& ctx ) override;
	bool on_pointer( const PointerEvent& e, UIContext& ctx ) override;

	void render( Renderer& renderer, const UIContext& ctx ) const override;
	void collect_hints( std::vector< Hint >& hints ) const override;

	/// 既定の表示（小数第 1 位まで）
	string_t format_value( float_t v ) const;

}; // class Slider

} // namespace core::ui
