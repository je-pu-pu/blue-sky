#include "SettingStore.h"

#include <game/Config.h>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace core::setting
{

namespace
{

/// 数値系の値を float に変換する（文字列は 0）
float_t to_float( const SettingStore::Value& v )
{
	if ( auto p = std::get_if< bool >( & v ) ) return *p ? 1.f : 0.f;
	if ( auto p = std::get_if< int >( & v ) ) return static_cast< float_t >( *p );
	if ( auto p = std::get_if< float_t >( & v ) ) return *p;
	return 0.f;
}

} // namespace

SettingStore::Entry* SettingStore::find( const string_t& key )
{
	auto i = index_.find( key );
	return i == index_.end() ? nullptr : & entries_[ i->second ];
}

const SettingStore::Entry* SettingStore::find( const string_t& key ) const
{
	auto i = index_.find( key );
	return i == index_.end() ? nullptr : & entries_[ i->second ];
}

SettingStore::Entry& SettingStore::add_entry( const string_t& key, const Value& def, ApplyMode mode, ApplyFn apply )
{
	if ( Entry* e = find( key ) )
	{
		// 同じキーの再登録は上書き（グループ所属とフックは維持）
		e->default_value = e->committed = e->pending = def;
		e->mode = mode;
		e->apply = std::move( apply );
		return *e;
	}

	Entry e;
	e.key = key;
	e.default_value = e.committed = e.pending = def;
	e.mode = mode;
	e.apply = std::move( apply );

	index_[ key ] = entries_.size();
	entries_.push_back( std::move( e ) );

	return entries_.back();
}

void SettingStore::add_bool( const string_t& key, bool def, ApplyMode mode, ApplyFn apply )
{
	add_entry( key, Value( def ), mode, std::move( apply ) );
}

void SettingStore::add_int( const string_t& key, int def, int min, int max, ApplyMode mode, ApplyFn apply )
{
	Entry& e = add_entry( key, Value( def ), mode, std::move( apply ) );
	e.min = static_cast< float_t >( min );
	e.max = static_cast< float_t >( max );
	e.has_range = true;
	e.default_value = e.committed = e.pending = normalize( e, Value( def ) );
}

void SettingStore::add_float( const string_t& key, float_t def, float_t min, float_t max, float_t step, ApplyMode mode, ApplyFn apply )
{
	Entry& e = add_entry( key, Value( def ), mode, std::move( apply ) );
	e.min = min;
	e.max = max;
	e.step = step;
	e.has_range = true;
	e.default_value = e.committed = e.pending = normalize( e, Value( def ) );
}

void SettingStore::add_string( const string_t& key, const string_t& def, ApplyMode mode, ApplyFn apply )
{
	add_entry( key, Value( def ), mode, std::move( apply ) );
}

void SettingStore::add_apply_group( const std::vector< string_t >& keys, std::function< void() > apply )
{
	const int group_index = static_cast< int >( groups_.size() );
	groups_.push_back( Group{ keys, std::move( apply ) } );

	for ( const auto& key : keys )
	{
		if ( Entry* e = find( key ) )
		{
			e->group = group_index;
		}
	}
}

void SettingStore::set_on_set( const string_t& key, OnSetHook hook )
{
	if ( Entry* e = find( key ) )
	{
		e->on_set = std::move( hook );
	}
}

SettingStore::Value SettingStore::normalize( const Entry& e, const Value& v ) const
{
	// 文字列項目
	if ( std::holds_alternative< string_t >( e.default_value ) )
	{
		if ( auto p = std::get_if< string_t >( & v ) ) return *p;
		return e.default_value;
	}

	// 文字列を数値項目に入れようとした場合は無視（現在値のまま）
	if ( std::holds_alternative< string_t >( v ) )
	{
		return e.pending;
	}

	if ( std::holds_alternative< bool >( e.default_value ) )
	{
		return Value( to_float( v ) != 0.f );
	}

	float_t f = to_float( v );

	if ( ! std::isfinite( f ) )
	{
		return e.default_value;
	}

	if ( e.has_range )
	{
		if ( e.step > 0.f )
		{
			f = e.min + std::round( ( f - e.min ) / e.step ) * e.step;
		}

		f = std::clamp( f, e.min, e.max );
	}

	if ( std::holds_alternative< int >( e.default_value ) )
	{
		return Value( static_cast< int >( std::lround( f ) ) );
	}

	return Value( f );
}

bool SettingStore::equals( const Entry& e, const Value& a, const Value& b ) const
{
	if ( auto pa = std::get_if< float_t >( & a ) )
	{
		auto pb = std::get_if< float_t >( & b );

		if ( ! pb )
		{
			return false;
		}

		// step の半分未満の差は同じ値とみなす（step が無ければ相対誤差）
		const float_t eps = e.step > 0.f ? e.step * 0.5f : std::max( 1e-6f, std::abs( *pa ) * 1e-6f );
		return std::abs( *pa - *pb ) < eps;
	}

	return a == b;
}

SettingStore::Value SettingStore::get_value( const string_t& key ) const
{
	const Entry* e = find( key );
	return e ? e->pending : Value();
}

SettingStore::Value SettingStore::get_committed( const string_t& key ) const
{
	const Entry* e = find( key );
	return e ? e->committed : Value();
}

SettingStore::Value SettingStore::get_default( const string_t& key ) const
{
	const Entry* e = find( key );
	return e ? e->default_value : Value();
}

void SettingStore::set( const string_t& key, const Value& value )
{
	Entry* e = find( key );

	if ( ! e )
	{
		return;
	}

	const Value old_value = e->pending;
	const Value new_value = normalize( *e, value );

	if ( equals( *e, old_value, new_value ) )
	{
		return;
	}

	e->pending = new_value;

	if ( e->mode == ApplyMode::IMMEDIATE )
	{
		if ( e->apply )
		{
			e->apply( e->pending );
		}

		change_count_++;
	}

	if ( e->on_set && ! in_hook_ )
	{
		in_hook_ = true;
		// フックの中で entries_ が再確保されることは無い（登録はしない前提）が、念のためコピーで呼ぶ
		OnSetHook hook = e->on_set;
		hook( *this, key, old_value );
		in_hook_ = false;
	}
}

bool SettingStore::is_pending( const string_t& key ) const
{
	const Entry* e = find( key );
	return e && ! equals( *e, e->pending, e->committed );
}

bool SettingStore::has_pending_changes() const
{
	for ( const auto& e : entries_ )
	{
		if ( ! equals( e, e.pending, e.committed ) ) return true;
	}

	return false;
}

bool SettingStore::has_pending_deferred() const
{
	for ( const auto& e : entries_ )
	{
		if ( e.mode == ApplyMode::DEFERRED && ! equals( e, e.pending, e.committed ) ) return true;
	}

	return false;
}

void SettingStore::commit_immediate()
{
	for ( auto& e : entries_ )
	{
		if ( e.mode == ApplyMode::IMMEDIATE )
		{
			e.committed = e.pending;
		}
	}
}

void SettingStore::apply_changed( const std::vector< size_t >& changed )
{
	std::vector< bool > group_dirty( groups_.size(), false );

	for ( size_t i : changed )
	{
		const Entry& e = entries_[ i ];

		if ( e.group >= 0 )
		{
			group_dirty[ e.group ] = true;
		}
		else if ( e.apply )
		{
			e.apply( e.pending );
		}
	}

	for ( size_t g = 0; g < groups_.size(); g++ )
	{
		if ( group_dirty[ g ] && groups_[ g ].apply )
		{
			groups_[ g ].apply();
		}
	}

	if ( ! changed.empty() )
	{
		change_count_++;
	}
}

bool SettingStore::apply_deferred()
{
	std::vector< size_t > changed;

	for ( size_t i = 0; i < entries_.size(); i++ )
	{
		Entry& e = entries_[ i ];

		if ( e.mode == ApplyMode::DEFERRED && ! equals( e, e.pending, e.committed ) )
		{
			e.committed = e.pending;
			changed.push_back( i );
		}
	}

	apply_changed( changed );

	return ! changed.empty();
}

void SettingStore::revert()
{
	std::vector< size_t > reapply;

	for ( size_t i = 0; i < entries_.size(); i++ )
	{
		Entry& e = entries_[ i ];

		if ( ! equals( e, e.pending, e.committed ) )
		{
			e.pending = e.committed;

			// IMMEDIATE は既に反映済みなので、確定値で反映し直す
			if ( e.mode == ApplyMode::IMMEDIATE )
			{
				reapply.push_back( i );
			}
		}
	}

	apply_changed( reapply );
}

void SettingStore::revert_deferred()
{
	for ( auto& e : entries_ )
	{
		if ( e.mode == ApplyMode::DEFERRED )
		{
			e.pending = e.committed;
		}
	}
}

void SettingStore::reset_to_defaults()
{
	// フックは呼ばない（既定値どうしは整合している前提。途中状態で入れ替えが起きるのを避ける）
	std::vector< size_t > reapply;

	for ( size_t i = 0; i < entries_.size(); i++ )
	{
		Entry& e = entries_[ i ];

		if ( ! equals( e, e.pending, e.default_value ) )
		{
			e.pending = e.default_value;

			if ( e.mode == ApplyMode::IMMEDIATE )
			{
				reapply.push_back( i );
			}
		}
	}

	apply_changed( reapply );
}

SettingStore::Snapshot SettingStore::snapshot() const
{
	Snapshot s;

	for ( const auto& e : entries_ )
	{
		s[ e.key ] = e.committed;
	}

	return s;
}

void SettingStore::rollback( const Snapshot& snapshot, const std::vector< string_t >& keys )
{
	std::vector< size_t > changed;

	for ( const auto& key : keys )
	{
		auto si = snapshot.find( key );
		auto ii = index_.find( key );

		if ( si == snapshot.end() || ii == index_.end() )
		{
			continue;
		}

		Entry& e = entries_[ ii->second ];
		const bool differs = ! equals( e, e.pending, si->second ) || ! equals( e, e.committed, si->second );

		e.committed = e.pending = si->second;

		if ( differs )
		{
			changed.push_back( ii->second );
		}
	}

	apply_changed( changed );
}

void SettingStore::sync_from_runtime( const string_t& key, const Value& value )
{
	Entry* e = find( key );

	if ( ! e )
	{
		return;
	}

	const Value v = normalize( *e, value );

	if ( ! equals( *e, e->committed, v ) || ! equals( *e, e->pending, v ) )
	{
		e->committed = e->pending = v;
		change_count_++;
	}
}

string_t SettingStore::to_text( const Value& v )
{
	if ( auto p = std::get_if< bool >( & v ) ) return *p ? "1" : "0";	// 既存の設定ファイル（0/1）と互換
	if ( auto p = std::get_if< int >( & v ) ) return std::to_string( *p );
	if ( auto p = std::get_if< string_t >( & v ) ) return *p;

	std::ostringstream ss;
	ss << std::setprecision( 6 ) << std::get< float_t >( v );
	return ss.str();
}

bool SettingStore::parse_text( const Value& type_sample, const string_t& text, Value& out )
{
	if ( std::holds_alternative< string_t >( type_sample ) )
	{
		out = text;
		return true;
	}

	if ( text.empty() )
	{
		return false;
	}

	if ( std::holds_alternative< bool >( type_sample ) )
	{
		if ( text == "true" ) { out = true; return true; }
		if ( text == "false" ) { out = false; return true; }
	}

	// 数値として解釈（末尾にゴミがあれば失敗）
	std::istringstream ss( text );
	double d = 0.0;
	ss >> d;

	if ( ss.fail() || ! ( ss >> std::ws ).eof() )
	{
		return false;
	}

	out = static_cast< float_t >( d );
	return true;
}

void SettingStore::load( const game::Config& config )
{
	for ( auto& e : entries_ )
	{
		const string_t text = config.get< string_t >( e.key.c_str(), string_t() );
		Value parsed;

		if ( ! text.empty() && parse_text( e.default_value, text, parsed ) )
		{
			e.committed = e.pending = normalize( e, parsed );
		}
		else
		{
			e.committed = e.pending = e.default_value;
		}
	}

	change_count_++;
}

void SettingStore::save( game::Config& config ) const
{
	for ( const auto& e : entries_ )
	{
		config.set< string_t >( e.key.c_str(), to_text( e.committed ) );
	}
}

void SettingStore::apply_all_individual()
{
	for ( const auto& e : entries_ )
	{
		if ( e.group < 0 && e.apply )
		{
			e.apply( e.committed );
		}
	}

	change_count_++;
}

} // namespace core::setting
