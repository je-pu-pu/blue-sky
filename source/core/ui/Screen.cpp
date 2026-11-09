#include "Screen.h"
#include "Dialog.h"
#include "Renderer.h"
#include "StringTable.h"
#include "Theme.h"

#include <algorithm>
#include <cmath>

namespace core::ui
{

namespace
{

Axis axis_of( NavDir d )
{
	return ( d == NavDir::UP || d == NavDir::DOWN ) ? Axis::VERTICAL : Axis::HORIZONTAL;
}

bool is_forward( NavDir d )
{
	return d == NavDir::DOWN || d == NavDir::RIGHT;
}

/// root 以下のツリーに target が含まれるか（target は破棄済みかもしれないので参照しない。ポインタ比較のみ）
bool tree_contains( const Widget* root, const Widget* target, size_t ( *count )( const Widget* ), const Widget* ( *at )( const Widget*, size_t ) )
{
	if ( root == target )
	{
		return true;
	}

	for ( size_t i = 0; i < count( root ); i++ )
	{
		if ( tree_contains( at( root, i ), target, count, at ) )
		{
			return true;
		}
	}

	return false;
}

const char_t* hint_action_name( HintAction a )
{
	switch ( a )
	{
		case HintAction::CONFIRM: return "confirm";
		case HintAction::CHANGE: return "change";
		case HintAction::CANCEL: return "cancel";
		default: return "tab";
	}
}

} // namespace

// 全ての子（公開されていない TabView のページも含む）を辿るためのアクセサ
struct WidgetAccess
{
	static size_t all_count( const Widget* w ) { return w->children_.size(); }
	static const Widget* all_at( const Widget* w, size_t i ) { return w->children_[ i ].get(); }
};

Screen::Screen( const Theme& theme, const StringTable& strings, const Renderer& renderer )
	: theme_( theme )
	, strings_( strings )
	, renderer_( renderer )
{
}

Screen::~Screen() = default;

UIContext Screen::make_context()
{
	return UIContext{ theme_, strings_, renderer_, *this };
}

UIContext Screen::make_context() const
{
	return UIContext{ theme_, strings_, renderer_, const_cast< Screen& >( *this ) };
}

// ---- 層 ----

Widget* Screen::set_root( std::unique_ptr< Widget > root, std::function< void() > on_cancel )
{
	Widget* raw = root.get();

	Layer layer;
	layer.root = std::move( root );
	layer.on_cancel = std::move( on_cancel );

	if ( layers_.empty() )
	{
		layers_.push_back( std::move( layer ) );
	}
	else
	{
		layers_.front() = std::move( layer );
	}

	pointer_capture_ = nullptr;
	key_capture_ = nullptr;
	layout_dirty_ = true;
	layout();
	focus_initial( layers_.front() );

	return raw;
}

void Screen::push_layer( std::unique_ptr< Widget > root, std::function< void() > on_cancel, bool dim )
{
	// std::function はコピー可能である必要があるので shared_ptr に包む
	auto holder = std::make_shared< std::unique_ptr< Widget > >( std::move( root ) );

	post( [this, holder, on_cancel, dim]
	{
		Layer layer;
		layer.root = std::move( *holder );
		layer.on_cancel = on_cancel;
		layer.dim = dim;
		layers_.push_back( std::move( layer ) );

		pointer_capture_ = nullptr;
		layout_dirty_ = true;
		layout();
		focus_initial( layers_.back() );
	} );
}

void Screen::close_layer( const Widget* root )
{
	post( [this, root]
	{
		for ( size_t i = 0; i < layers_.size(); i++ )
		{
			if ( layers_[ i ].root.get() == root )
			{
				layers_.erase( layers_.begin() + static_cast< std::ptrdiff_t >( i ) );
				layout_dirty_ = true;
				return;
			}
		}
	} );
}

void Screen::show_dialog( DialogSpec spec )
{
	auto dialog = std::make_unique< Dialog >( std::move( spec ) );
	Dialog* raw = dialog.get();
	raw->set_screen( this );

	push_layer( std::move( dialog ), [raw] { raw->press_cancel(); }, true );
}

void Screen::post( std::function< void() > command )
{
	commands_.push_back( std::move( command ) );
}

void Screen::run_commands()
{
	if ( commands_.empty() )
	{
		return;
	}

	// 実行中に積まれたコマンドも同じフレームで最後まで処理する（添字ループ）
	for ( size_t i = 0; i < commands_.size(); i++ )
	{
		auto command = std::move( commands_[ i ] );	// 実行中に vector が再確保されても安全なようにコピーしてから呼ぶ
		command();
	}

	commands_.clear();

	// 破棄された部品を指していないか確認する
	auto alive = [this] ( const Widget* w )
	{
		for ( const auto& layer : layers_ )
		{
			if ( tree_contains( layer.root.get(), w, & WidgetAccess::all_count, & WidgetAccess::all_at ) ) return true;
		}
		return false;
	};

	if ( pointer_capture_ && ! alive( pointer_capture_ ) ) pointer_capture_ = nullptr;
	if ( key_capture_ && ! alive( key_capture_ ) ) key_capture_ = nullptr;
}

// ---- フォーカス ----

Widget* Screen::get_focused() const
{
	const Layer* layer = top_layer();
	return layer ? layer->focused : nullptr;
}

void Screen::set_focus( Widget* w, FocusReason reason )
{
	if ( ! w )
	{
		return;
	}

	Layer* layer = nullptr;

	for ( auto i = layers_.rbegin(); i != layers_.rend(); ++i )
	{
		if ( tree_contains( i->root.get(), w, & WidgetAccess::all_count, & WidgetAccess::all_at ) )
		{
			layer = & *i;
			break;
		}
	}

	if ( ! layer || layer->focused == w )
	{
		return;
	}

	if ( layer->focused && tree_contains( layer->root.get(), layer->focused, & WidgetAccess::all_count, & WidgetAccess::all_at ) )
	{
		layer->focused->focused_ = false;
	}

	layer->focused = w;
	w->focused_ = true;

	// 祖先に「最後にフォーカスした子」を記録する
	for ( Widget* c = w, * p = w->parent_; p; c = p, p = p->parent_ )
	{
		p->last_focused_child_ = c;
	}

	// ポインタ由来では自動スクロールしない（カーソルの下で内容が動いて暴れるため）
	if ( reason != FocusReason::POINTER )
	{
		for ( Widget* p = w->parent_; p; p = p->parent_ )
		{
			p->ensure_visible( w->rect_ );
		}

		layout_dirty_ = true;
	}
}

void Screen::focus_into( Widget* container, FocusReason reason )
{
	if ( Widget* target = find_entry( container, NavDir::DOWN, nullptr ) )
	{
		set_focus( target, reason );
	}
}

Widget* Screen::find_first_focusable( Widget* w ) const
{
	if ( ! w || ! w->is_visible() )
	{
		return nullptr;
	}

	if ( w->can_focus() )
	{
		return w;
	}

	for ( size_t i = 0; i < w->child_count(); i++ )
	{
		if ( Widget* f = find_first_focusable( w->child_at( i ) ) )
		{
			return f;
		}
	}

	return nullptr;
}

void Screen::focus_initial( Layer& layer )
{
	Widget* target = nullptr;

	if ( auto* dialog = dynamic_cast< Dialog* >( layer.root.get() ) )
	{
		target = dialog->get_default_button();
	}

	if ( ! target || ! target->can_focus() )
	{
		target = find_first_focusable( layer.root.get() );
	}

	if ( target )
	{
		set_focus( target, FocusReason::PROGRAM );
	}
}

void Screen::repair_focus()
{
	Layer* layer = top_layer();

	if ( ! layer )
	{
		return;
	}

	Widget* focused = layer->focused;

	if ( focused && ! tree_contains( layer->root.get(), focused, & WidgetAccess::all_count, & WidgetAccess::all_at ) )
	{
		// 破棄された
		layer->focused = nullptr;
		focused = nullptr;
	}

	if ( focused && focused->can_focus() )
	{
		return;
	}

	// ツリー順の一覧を作り、元の位置に最も近いフォーカス可能な部品へ移す
	std::vector< Widget* > order;
	std::function< void( Widget* ) > walk = [&] ( Widget* w )
	{
		order.push_back( w );
		for ( size_t i = 0; i < WidgetAccess::all_count( w ); i++ )
		{
			walk( const_cast< Widget* >( WidgetAccess::all_at( w, i ) ) );
		}
	};
	walk( layer->root.get() );

	int origin = -1;

	for ( size_t i = 0; i < order.size(); i++ )
	{
		if ( order[ i ] == focused ) { origin = static_cast< int >( i ); break; }
	}

	Widget* best = nullptr;

	if ( origin >= 0 )
	{
		for ( int d = 1; d < static_cast< int >( order.size() ) && ! best; d++ )
		{
			// 後ろ優先
			const int after = origin + d;
			const int before = origin - d;

			if ( after < static_cast< int >( order.size() ) && order[ after ]->can_focus() ) best = order[ after ];
			else if ( before >= 0 && order[ before ]->can_focus() ) best = order[ before ];
		}
	}
	else
	{
		best = find_first_focusable( layer->root.get() );
	}

	if ( focused )
	{
		focused->focused_ = false;
	}

	layer->focused = nullptr;

	if ( best )
	{
		set_focus( best, FocusReason::PROGRAM );
	}
}

// ---- ナビゲーション ----

Widget* Screen::find_entry( Widget* w, NavDir dir, const Widget* from ) const
{
	if ( ! w || ! w->is_visible() || ! w->is_enabled() )
	{
		return nullptr;
	}

	if ( w->can_focus() )
	{
		return w;
	}

	const size_t n = w->child_count();

	if ( n == 0 )
	{
		return nullptr;
	}

	// 1. 最後にフォーカスした子を優先
	const int remembered = w->last_focused_child_ ? w->index_of_child( w->last_focused_child_ ) : -1;

	if ( remembered >= 0 )
	{
		if ( Widget* e = find_entry( w->child_at( static_cast< size_t >( remembered ) ), dir, from ) )
		{
			return e;
		}
	}

	// 2. 並び順を決める
	std::vector< size_t > order;

	for ( size_t i = 0; i < n; i++ ) order.push_back( i );

	if ( w->nav_axis() == axis_of( dir ) )
	{
		// 同じ軸のコンテナには端から入る
		if ( ! is_forward( dir ) )
		{
			std::reverse( order.begin(), order.end() );
		}
	}
	else if ( w->nav_axis() != Axis::NONE && from )
	{
		// 直交するコンテナには、直前の位置に近い子から入る
		const bool vertical = w->nav_axis() == Axis::VERTICAL;
		const float_t ref = vertical ? from->rect().center_y() : from->rect().center_x();

		std::stable_sort( order.begin(), order.end(), [&] ( size_t a, size_t b )
		{
			const Rect& ra = w->child_at( a )->rect();
			const Rect& rb = w->child_at( b )->rect();
			const float_t da = std::abs( ( vertical ? ra.center_y() : ra.center_x() ) - ref );
			const float_t db = std::abs( ( vertical ? rb.center_y() : rb.center_x() ) - ref );
			return da < db;
		} );
	}

	for ( size_t i : order )
	{
		if ( Widget* e = find_entry( w->child_at( i ), dir, from ) )
		{
			return e;
		}
	}

	return nullptr;
}

Widget* Screen::find_nav_target( Widget* from, NavDir dir ) const
{
	const Axis axis = axis_of( dir );
	const int step = is_forward( dir ) ? 1 : -1;

	// 祖先を遡り、同じ軸のコンテナの中で次の兄弟を探す
	for ( Widget* child = from, * p = from->parent_; p; child = p, p = p->parent_ )
	{
		if ( p->nav_axis() != axis )
		{
			continue;
		}

		const int index = p->index_of_child( child );

		if ( index < 0 )
		{
			continue;
		}

		for ( int i = index + step; i >= 0 && i < static_cast< int >( p->child_count() ); i += step )
		{
			if ( Widget* e = find_entry( p->child_at( static_cast< size_t >( i ) ), dir, from ) )
			{
				return e;
			}
		}
	}

	// どこへも抜けられなかったときだけ、wrap 指定のコンテナの中で反対端へ回る
	for ( Widget* child = from, * p = from->parent_; p; child = p, p = p->parent_ )
	{
		if ( p->nav_axis() != axis || ! p->nav_wrap() )
		{
			continue;
		}

		const int index = p->index_of_child( child );
		const int n = static_cast< int >( p->child_count() );

		if ( index < 0 )
		{
			continue;
		}

		for ( int k = 0; k < n; k++ )
		{
			const int i = step > 0 ? k : n - 1 - k;

			if ( i == index )
			{
				break;
			}

			if ( Widget* e = find_entry( p->child_at( static_cast< size_t >( i ) ), dir, from ) )
			{
				return e;
			}
		}
	}

	return nullptr;
}

// ---- 入力 ----

Widget* Screen::hit_test( Widget* w, float_t x, float_t y, const Rect* clip ) const
{
	if ( ! w || ! w->is_visible() )
	{
		return nullptr;
	}

	// 表示範囲（ScrollView）を子孫へ引き継ぐ。入れ子なら交差
	Rect own_clip;
	const Rect* child_clip = clip;

	if ( const Rect* c = w->get_view_clip() )
	{
		own_clip = clip ? clip->intersection( *c ) : *c;
		child_clip = & own_clip;
	}

	// 範囲の外側の点は、範囲内の子孫には当たらないので子は調べない
	if ( ! child_clip || child_clip->contains( x, y ) )
	{
		for ( size_t i = w->child_count(); i > 0; i-- )
		{
			if ( Widget* hit = hit_test( w->child_at( i - 1 ), x, y, child_clip ) )
			{
				return hit;
			}
		}
	}

	// 自分自身 : 範囲に完全に収まっていなければ当たらない
	if ( clip && ! Widget::is_inside( w->rect(), *clip ) )
	{
		return nullptr;
	}

	if ( ( w->is_focusable_type() || w->accepts_pointer() ) && w->is_enabled() && w->rect().contains( x, y ) )
	{
		return w;
	}

	return nullptr;
}

void Screen::handle_pointer( const UIInput& in, UIContext& ctx )
{
	Layer* layer = top_layer();
	const bool moved = in.pointer_moved || in.pointer_x != last_pointer_x_ || in.pointer_y != last_pointer_y_;

	last_pointer_x_ = in.pointer_x;
	last_pointer_y_ = in.pointer_y;

	if ( moved )
	{
		hover_enabled_ = true;
	}

	// ドラッグ
	if ( pointer_capture_ && in.pointer_down && moved )
	{
		pointer_capture_->on_pointer( PointerEvent{ PointerEvent::Type::DRAG, in.pointer_x, in.pointer_y, pointer_capture_->rect().contains( in.pointer_x, in.pointer_y ) }, ctx );
	}

	Widget* hit = hit_test( layer->root.get(), in.pointer_x, in.pointer_y );

	// ホバー（ドラッグ中はしない）
	if ( moved && hover_enabled_ && ! pointer_capture_ && hit && hit->can_focus() && hit != layer->focused )
	{
		set_focus( hit, FocusReason::POINTER );
		play( UISoundEvent::MOVE );
	}

	if ( in.pointer_pressed && hit )
	{
		if ( hit->can_focus() )
		{
			set_focus( hit, FocusReason::POINTER );
		}

		pointer_capture_ = hit;
		hit->on_pointer( PointerEvent{ PointerEvent::Type::PRESS, in.pointer_x, in.pointer_y, true }, ctx );
	}

	if ( in.pointer_released && pointer_capture_ )
	{
		Widget* target = pointer_capture_;
		pointer_capture_ = nullptr;
		target->on_pointer( PointerEvent{ PointerEvent::Type::RELEASE, in.pointer_x, in.pointer_y, target->rect().contains( in.pointer_x, in.pointer_y ) }, ctx );
	}

	// ホイール : カーソル下の部品から祖先へ、無ければフォーカス部品から
	if ( in.wheel != 0 )
	{
		bool used = false;

		for ( Widget* w = hit; w && ! used; w = w->parent_ ) used = w->on_wheel( in.wheel, ctx );
		for ( Widget* w = layer->focused; w && ! used; w = w->parent_ ) used = w->on_wheel( in.wheel, ctx );

		if ( used )
		{
			hover_enabled_ = false;
		}
	}
}

void Screen::handle_nav( NavDir dir, UIContext& ctx )
{
	Layer* layer = top_layer();
	Widget* focused = layer->focused;

	if ( focused && focused->on_nav( dir, ctx ) )
	{
		return;
	}

	Widget* target = focused ? find_nav_target( focused, dir ) : find_first_focusable( layer->root.get() );

	if ( target && target != focused )
	{
		set_focus( target, FocusReason::NAV );
		play( UISoundEvent::MOVE );
	}
}

void Screen::handle_cancel( UIContext& ctx )
{
	Layer* layer = top_layer();

	if ( layer->focused && layer->focused->on_cancel( ctx ) )
	{
		return;
	}

	if ( layer->on_cancel )
	{
		auto on_cancel = layer->on_cancel;	// 層が閉じられても安全なようにコピー
		play( UISoundEvent::CANCEL );
		on_cancel();
	}
}

void Screen::update( const UIInput& input, float_t dt )
{
	if ( layers_.empty() )
	{
		run_commands();
		return;
	}

	layout();

	UIContext ctx = make_context();

	if ( suppress_frames_ > 0 )
	{
		suppress_frames_--;
		last_pointer_x_ = input.pointer_x;
		last_pointer_y_ = input.pointer_y;
	}
	else
	{
		last_device_ = input.last_device;

		if ( key_capture_ )
		{
			// キー待ち中は「中止」と「キー入力」だけ。待ちを始めたフレームのキーは無視
			if ( ! key_capture_started_ )
			{
				if ( input.cancel )
				{
					key_capture_->on_cancel( ctx );
				}
				else if ( input.pushed_key != 0 )
				{
					key_capture_->on_key_captured( input.pushed_key, ctx );
				}
			}
		}
		else
		{
			handle_pointer( input, ctx );

			if ( input.tab_prev || input.tab_next )
			{
				const int d = input.tab_next ? 1 : -1;
				bool used = false;

				for ( Widget* w = get_focused(); w && ! used; w = w->parent_ ) used = w->on_tab( d, ctx );

				if ( ! used )
				{
					std::function< bool( Widget* ) > walk = [&] ( Widget* w ) -> bool
					{
						if ( w->on_tab( d, ctx ) ) return true;
						for ( size_t i = 0; i < w->child_count(); i++ ) if ( walk( w->child_at( i ) ) ) return true;
						return false;
					};
					walk( top_layer()->root.get() );
				}
			}

			for ( NavDir d : { NavDir::UP, NavDir::DOWN, NavDir::LEFT, NavDir::RIGHT } )
			{
				if ( input.nav( d ) )
				{
					handle_nav( d, ctx );
				}
			}

			if ( input.confirm && get_focused() )
			{
				get_focused()->on_confirm( ctx );
			}

			if ( input.cancel )
			{
				handle_cancel( ctx );
			}
		}
	}

	key_capture_started_ = false;

	for ( auto& layer : layers_ )
	{
		layer.root->update( dt, ctx );
	}

	run_commands();
	layout();
	repair_focus();
}

void Screen::layout()
{
	if ( ! layout_dirty_ )
	{
		return;
	}

	layout_dirty_ = false;

	const UIContext ctx = make_context();
	const Rect screen_rect( 0.f, 0.f, renderer_.get_screen_width(), renderer_.get_screen_height() );

	for ( auto& layer : layers_ )
	{
		layer.root->arrange( screen_rect, ctx );
	}
}

void Screen::render( Renderer& renderer )
{
	layout();

	const UIContext ctx = make_context();

	for ( const auto& layer : layers_ )
	{
		if ( layer.dim )
		{
			renderer.fill_screen( theme_.dialog_dim );
		}

		layer.root->render( renderer, ctx );
	}
}

// ---- キー割り当て待ち ----

void Screen::begin_key_capture( Widget* w )
{
	key_capture_ = w;
	key_capture_started_ = true;
}

void Screen::end_key_capture()
{
	key_capture_ = nullptr;
}

// ---- ヒント・説明 ----

string_t Screen::get_description() const
{
	const Widget* f = get_focused();

	if ( ! f || f->get_description_key().empty() )
	{
		return string_t();
	}

	return strings_.get( f->get_description_key() );
}

string_t Screen::build_hint_text() const
{
	const Layer* layer = top_layer();

	if ( ! layer )
	{
		return string_t();
	}

	std::vector< Hint > hints;

	if ( key_capture_ )
	{
		hints.push_back( Hint{ HintAction::CANCEL, "hint.abort" } );
	}
	else
	{
		if ( layer->focused )
		{
			layer->focused->collect_hints( hints );
		}

		// 層全体のヒント（タブ）
		bool has_tabs = false;
		std::function< void( const Widget* ) > walk = [&] ( const Widget* w )
		{
			if ( w->has_tabs() ) has_tabs = true;
			for ( size_t i = 0; i < w->child_count() && ! has_tabs; i++ ) walk( w->child_at( i ) );
		};
		walk( layer->root.get() );

		if ( has_tabs )
		{
			hints.push_back( Hint{ HintAction::TAB, "hint.tab" } );
		}

		if ( layer->on_cancel )
		{
			hints.push_back( Hint{ HintAction::CANCEL, "hint.back" } );
		}
	}

	const string_t prefix = last_device_ == InputDevice::GAMEPAD ? "hint.pad." : "hint.key.";
	string_t text;
	std::vector< HintAction > used;

	for ( const auto& h : hints )
	{
		if ( std::find( used.begin(), used.end(), h.action ) != used.end() )
		{
			continue;
		}

		used.push_back( h.action );

		if ( ! text.empty() )
		{
			text += "    ";
		}

		text += strings_.get( prefix + hint_action_name( h.action ) );
		text += " ";
		text += strings_.get( h.text_key );
	}

	return text;
}

} // namespace core::ui
