#pragma once

#include <core/ui/LabeledRow.h>

#include <functional>

namespace core::ui
{

/**
 * キー割り当ての 1 行（「ジャンプ　[SPACE]」）
 *
 * 決定（またはクリック）で「キーを押してください」状態になり、次に押されたキーを setter に渡す。
 * キャンセル（Esc / パッド B）で中止する。中止しても画面は閉じない。
 * キーコードの意味は知らない（表示名は name_of で外から与える）。
 */
class KeyBind : public LabeledRow
{
public:
	/// 捕捉が外れた（層が積まれたなど）: 押している・ドラッグ中の状態を解除する
	void on_capture_lost() override { pressed_ = false; waiting_ = false; }

	using Getter = std::function< int() >;
	using Setter = std::function< void( int ) >;
	using NameOf = std::function< string_t( int ) >;

private:
	Getter getter_;
	Setter setter_;
	NameOf name_of_;
	bool waiting_ = false;
	bool pressed_ = false;

	void start( UIContext& ctx );

public:
	KeyBind( const string_t& label_key, Getter getter, Setter setter, NameOf name_of )
		: LabeledRow( label_key ), getter_( std::move( getter ) ), setter_( std::move( setter ) ), name_of_( std::move( name_of ) ) { }

	bool is_waiting() const { return waiting_; }
	AccessibleInfo describe( const UIContext& ctx ) const override;

	bool on_confirm( UIContext& ctx ) override;
	bool on_cancel( UIContext& ctx ) override;
	bool on_pointer( const PointerEvent& e, UIContext& ctx ) override;
	bool on_key_captured( int code, UIContext& ctx ) override;

	void render( Renderer& renderer, const UIContext& ctx ) const override;
	void collect_hints( std::vector< Hint >& hints ) const override;

}; // class KeyBind

} // namespace core::ui
