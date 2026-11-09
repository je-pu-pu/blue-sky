#include "Screen.h"
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

/// 組み込みのアクションの、操作ヒントの名前（hint.key.<名前> / hint.pad.<名前> を引く）
const char_t* builtin_action_name( ActionId a )
{
	switch ( a )
	{
		case ui_action::CONFIRM:		return "confirm";
		case ui_action::HINT_CHANGE:	return "change";
		case ui_action::CANCEL:			return "cancel";
		case ui_action::HINT_TAB:		return "tab";
		default:						return nullptr;
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

	// 差し替える前に、捕捉していた部品に捕捉が外れたことを知らせる
	release_captures();

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

	layout_dirty_ = true;
	layout();
	focus_initial( layers_.front() );

	return raw;
}

void Screen::push_layer( std::unique_ptr< Widget > root, std::function< void() > on_cancel, bool dim, bool modal )
{
	// std::function はコピー可能である必要があるので shared_ptr に包む
	auto holder = std::make_shared< std::unique_ptr< Widget > >( std::move( root ) );

	post( [this, holder, on_cancel, dim, modal]
	{
		Layer layer;
		layer.root = std::move( *holder );
		layer.on_cancel = on_cancel;
		layer.dim = dim;
		layer.modal = modal;

		// 表示だけの層は、下の層の操作（押している・キー待ち）を妨げない
		if ( ! modal )
		{
			layers_.push_back( std::move( layer ) );
			layout_dirty_ = true;
			layout();
			return;
		}

		// 下の層の部品の「押している」「ドラッグ中」「キー待ち」を解除する
		// （以前は Screen 側の捕捉を消すだけで、ボタンが押された見た目のまま残り、キー待ちも背面に残った）
		release_captures();

		layers_.push_back( std::move( layer ) );

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

void Screen::release_captures()
{
	if ( pointer_capture_ )
	{
		Widget* w = pointer_capture_;
		pointer_capture_ = nullptr;
		w->on_capture_lost();
	}

	if ( key_capture_ )
	{
		Widget* w = key_capture_;
		key_capture_ = nullptr;
		w->on_capture_lost();
	}
}

void Screen::register_action( ActionId action, const string_t& hint_name )
{
	action_names_[ action ] = hint_name;
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
		layer->focused->on_focus_changed( false, reason );
	}

	layer->focused = w;
	w->focused_ = true;
	w->on_focus_changed( true, reason );

	if ( focus_listener_ )
	{
		focus_listener_( w, reason );
	}

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
	// 最初のフォーカス先は層の部品が決める（Dialog は既定のボタン）。Screen は具体的な部品の種類を知らない
	Widget* target = layer.root->get_initial_focus();

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

	// 祖先を遡り、同じ軸のコンテナ（または独自の移動を持つコンテナ）の中で次の兄弟を探す
	for ( Widget* child = from, * p = from->parent_; p; child = p, p = p->parent_ )
	{
		// 独自の移動（Grid など）: コンテナが次の子を決める。見つからなければ祖先へ抜ける
		if ( const int index = p->index_of_child( child ); index >= 0 && p->nav_next_index( index, dir ) >= 0 )
		{
			for ( int i = p->nav_next_index( index, dir ); i >= 0; i = p->nav_next_index( i, dir ) )
			{
				if ( Widget* e = find_entry( p->child_at( static_cast< size_t >( i ) ), dir, from ) )
				{
					return e;
				}
			}

			continue;
		}

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

bool Screen::dispatch_action( ActionId action, UIContext& ctx )
{
	// フォーカス中の部品から親へ順に渡す（入れ子のメニューやリストが、子の使わなかった操作を受け取れる）
	for ( Widget* w = get_focused(); w; w = w->parent_ )
	{
		if ( w->on_action( action, ctx ) )
		{
			return true;
		}
	}

	// 誰も使わなかった操作の既定の処理
	switch ( action )
	{
		case ui_action::NAV_UP:		handle_nav( NavDir::UP, ctx ); return true;
		case ui_action::NAV_DOWN:	handle_nav( NavDir::DOWN, ctx ); return true;
		case ui_action::NAV_LEFT:	handle_nav( NavDir::LEFT, ctx ); return true;
		case ui_action::NAV_RIGHT:	handle_nav( NavDir::RIGHT, ctx ); return true;
		case ui_action::CANCEL:		handle_cancel( ctx ); return true;

		case ui_action::TAB_PREV:
		case ui_action::TAB_NEXT:
		{
			// フォーカスの外にあるタブ（画面に 1 つだけのタブなど）にも届くよう、層の中を探す
			const int d = action == ui_action::TAB_NEXT ? 1 : -1;

			std::function< bool( Widget* ) > walk = [&] ( Widget* w ) -> bool
			{
				if ( w->on_tab( d, ctx ) ) return true;
				for ( size_t i = 0; i < w->child_count(); i++ ) if ( walk( w->child_at( i ) ) ) return true;
				return false;
			};

			return walk( top_layer()->root.get() );
		}

		default:
			return false;
	}
}

void Screen::handle_nav( NavDir dir, UIContext& )
{
	Layer* layer = top_layer();
	Widget* focused = layer->focused;

	Widget* target = focused ? find_nav_target( focused, dir ) : find_first_focusable( layer->root.get() );

	if ( target && target != focused )
	{
		set_focus( target, FocusReason::NAV );
		play( UISoundEvent::MOVE );
	}
}

void Screen::handle_cancel( UIContext& )
{
	Layer* layer = top_layer();

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

			// 文字を受け取る部品がフォーカス中なら、文字はそちらへ。同じキーによるタブ切替（Q / E）は行わない
			Widget* focused = get_focused();
			const bool text_mode = focused && focused->wants_text_input();

			if ( text_mode && ! input.text.empty() )
			{
				focused->on_text( input.text, ctx );
			}

			if ( text_mode )
			{
				focused->on_composition( input.composition, ctx );
			}

			// 入力をアクションの列にして、順に処理する（タブ → 方向 → 決定 → キャンセル → 独自の操作）
			std::vector< ActionId > actions;

			if ( ! text_mode )
			{
				if ( input.tab_prev ) actions.push_back( ui_action::TAB_PREV );
				if ( input.tab_next ) actions.push_back( ui_action::TAB_NEXT );
			}

			if ( input.up ) actions.push_back( ui_action::NAV_UP );
			if ( input.down ) actions.push_back( ui_action::NAV_DOWN );
			if ( input.left ) actions.push_back( ui_action::NAV_LEFT );
			if ( input.right ) actions.push_back( ui_action::NAV_RIGHT );
			if ( input.confirm ) actions.push_back( ui_action::CONFIRM );
			if ( input.cancel ) actions.push_back( ui_action::CANCEL );

			actions.insert( actions.end(), input.actions.begin(), input.actions.end() );

			for ( ActionId a : actions )
			{
				dispatch_action( a, ctx );
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

Rect Screen::place_popup( const Rect& anchor, float_t width, float_t height, const Rect& bounds )
{
	const float_t w = std::min( width > 0.f ? width : anchor.w, bounds.w );
	const float_t h = std::min( height, bounds.h );

	// 下に入らなければ上へ。上にも入らなければ、範囲の下端に合わせる
	float_t y = anchor.bottom();

	if ( y + h > bounds.bottom() )
	{
		y = anchor.y - h >= bounds.y ? anchor.y - h : bounds.bottom() - h;
	}

	const float_t x = std::clamp( anchor.x, bounds.x, bounds.right() - w );

	return Rect( x, y, w, h );
}

void Screen::push_popup( std::unique_ptr< Widget > root, const Rect& anchor, float_t width, std::function< void() > on_cancel )
{
	auto holder = std::make_shared< std::unique_ptr< Widget > >( std::move( root ) );

	post( [this, holder, anchor, width, on_cancel]
	{
		Layer layer;
		layer.root = std::move( *holder );
		layer.on_cancel = on_cancel;
		layer.dim = false;
		layer.modal = true;
		layer.anchored = true;
		layer.anchor = anchor;
		layer.width = width;

		release_captures();
		layers_.push_back( std::move( layer ) );

		layout_dirty_ = true;
		layout();
		focus_initial( layers_.back() );
	} );
}

Rect Screen::get_safe_rect() const
{
	const float_t w = renderer_.get_screen_width();
	const float_t h = renderer_.get_screen_height();

	return Rect( 0.f, 0.f, w, h ).inset( w * theme_.safe_area, h * theme_.safe_area );
}

void Screen::layout()
{
	if ( ! layout_dirty_ )
	{
		return;
	}

	layout_dirty_ = false;

	const UIContext ctx = make_context();
	const Rect screen_rect = get_safe_rect();

	auto arrange_layer = [&] ( Layer& layer )
	{
		if ( layer.anchored )
		{
			const float_t w = std::min( layer.width > 0.f ? layer.width : layer.anchor.w, screen_rect.w );
			layer.root->arrange( place_popup( layer.anchor, w, layer.root->measure_height( w, ctx ), screen_rect ), ctx );
		}
		else
		{
			layer.root->arrange( screen_rect, ctx );
		}
	};

	for ( auto& layer : layers_ )
	{
		arrange_layer( layer );
	}

	// 見た目が変わって配置し直したときは、フォーカス中の部品が見えるようにスクロールしてから、もう一度配置する
	// （文字を大きくすると行が伸び、フォーカス中の行がスクロールの外に出ることがあるため）
	if ( reveal_focus_on_layout_ )
	{
		reveal_focus_on_layout_ = false;

		if ( Widget* f = get_focused() )
		{
			for ( Widget* p = f->parent_; p; p = p->parent_ )
			{
				p->ensure_visible( f->rect_ );
			}

			for ( auto& layer : layers_ )
			{
				arrange_layer( layer );
			}
		}
	}
}

void Screen::render( Renderer& renderer )
{
	layout();

	const UIContext ctx = make_context();

	// 積んだ描画は、関数を抜けるときに必ず描く（途中の return でも）
	struct FlushOnExit
	{
		Renderer& r;
		~FlushOnExit() { r.flush(); }
	} flush_on_exit { renderer };

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
	std::vector< ActionId > used;

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

		// 記号の名前 : 組み込み → register_action() で登録した名前 → 無ければ記号なし
		string_t name;

		if ( const char_t* builtin = builtin_action_name( h.action ) )
		{
			name = builtin;
		}
		else if ( auto i = action_names_.find( h.action ); i != action_names_.end() )
		{
			name = i->second;
		}

		if ( ! name.empty() )
		{
			text += strings_.get( prefix + name );
			text += " ";
		}
		text += strings_.get( h.text_key );
	}

	return text;
}

} // namespace core::ui
