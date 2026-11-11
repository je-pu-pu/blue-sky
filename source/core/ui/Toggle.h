#pragma once

#include <core/ui/LabeledRow.h>
#include <core/ui/ClickTracker.h>

#include <functional>

namespace core::ui
{

/**
 * オン／オフの切替
 *
 * 決定・左右・行のクリックで反転する。値は getter / setter で外部（設定ストア等）と結びつく。
 */
class Toggle : public LabeledRow
{
public:
	/// 捕捉が外れた（層が積まれたなど）: 押している・ドラッグ中の状態を解除する
	void on_capture_lost() override { click_.cancel(); }

	using Getter = std::function< bool() >;
	using Setter = std::function< void( bool ) >;

private:
	Getter getter_;
	Setter setter_;
	ClickTracker click_;

	void flip( UIContext& ctx );

public:
	Toggle( const string_t& label_key, Getter getter, Setter setter )
		: LabeledRow( label_key ), getter_( std::move( getter ) ), setter_( std::move( setter ) ) { }

	bool get_value() const { return getter_ ? getter_() : false; }
	AccessibleInfo describe( const UIContext& ctx ) const override;

	bool on_confirm( UIContext& ctx ) override;
	bool on_nav( NavDir dir, UIContext& ctx ) override;
	bool on_pointer( const PointerEvent& e, UIContext& ctx ) override;

	void render( Renderer& renderer, const UIContext& ctx ) const override;
	void collect_hints( std::vector< Hint >& hints ) const override;

}; // class Toggle

} // namespace core::ui
