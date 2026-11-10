#pragma once

#include <blue_sky/type.h>

#include <algorithm>
#include <map>

namespace blue_sky
{

/// シーンに渡す引数（キー → 値）。シーンは Scene::get_argument で読む
using SceneArguments = std::map< string_t, string_t >;

/**
 * "名前?キー=値&キー=値" をシーン名と引数に分ける（? が無ければ全体が名前で、引数は空）
 *
 * 例 : "options?tab=controls" → 名前 "options"・引数 { tab: controls }。= の無い語は値が空の引数になる
 */
inline void split_scene_name( const string_t& text, string_t& name, SceneArguments& args )
{
	const auto q = text.find( '?' );
	name = text.substr( 0, q );
	args.clear();

	if ( q == string_t::npos )
	{
		return;
	}

	size_t begin = q + 1;

	while ( begin <= text.size() )
	{
		const auto end = std::min( text.find( '&', begin ), text.size() );
		const string_t pair = text.substr( begin, end - begin );

		if ( ! pair.empty() )
		{
			const auto eq = pair.find( '=' );
			args[ pair.substr( 0, eq ) ] = eq == string_t::npos ? string_t() : pair.substr( eq + 1 );
		}

		begin = end + 1;
	}
}

/**
 * シーン遷移の要求
 *
 * 遷移は「要求」としてキューに積み、GameMain がフレームの境目でまとめて実行する。
 * （以前は Lua の scene() などがその場で実行し、実行中のシーンが破棄される危険があった）
 */
struct SceneRequest
{
	enum class Type
	{
		NONE,
		REPLACE_BASE,		///< ベースシーンを置き換える（上のオーバーレイはすべて閉じる）
		PUSH_OVERLAY,		///< オーバーレイを積む
		POP_OVERLAY,		///< 最前面のオーバーレイを閉じる
		REPLACE_OVERLAY,	///< 最前面のオーバーレイを置き換える
		QUIT,				///< アプリを終了する
	};

	Type type = Type::NONE;
	string_t name;			///< シーン名（REPLACE_BASE / PUSH_OVERLAY / REPLACE_OVERLAY）
	string_t stage_name;	///< ベースを置き換えるときのステージ名（空なら変えない）
	SceneArguments args;	///< 開くシーンに渡す引数（"push:options?tab=controls" の tab=controls。以前は遷移で引数を渡せなかった。B7）

	bool operator == ( const SceneRequest& r ) const { return type == r.type && name == r.name && stage_name == r.stage_name && args == r.args; }
};

/**
 * シーンが出す遷移の文字列（Scene::set_next_scene）を要求に変える
 *
 * - ""            : 要求なし
 * - "pop"         : 最前面のオーバーレイを閉じる（オーバーレイから出したときだけ）
 * - "push:<名前>" : オーバーレイを積む
 * - "base:<名前>" : ベースシーンを置き換える（オーバーレイから出してもベースを置き換える。ポーズ → ステージ選択へ など）
 * - "quit"        : 終了
 * - "<名前>"      : オーバーレイから出したら最前面の置き換え、ベースから出したらベースの置き換え
 *
 * 名前の後には "?キー=値&キー=値" で引数を付けられる（例 : "push:options?tab=controls"。split_scene_name）
 *
 * @param from_overlay オーバーレイが出した文字列か
 */
inline SceneRequest parse_scene_request( const string_t& text, bool from_overlay )
{
	SceneRequest r;

	if ( text.empty() )
	{
		return r;
	}

	const auto starts_with = [ &text ] ( const char_t* prefix ) { return text.rfind( prefix, 0 ) == 0; };

	if ( text == "pop" )
	{
		r.type = from_overlay ? SceneRequest::Type::POP_OVERLAY : SceneRequest::Type::NONE;
	}
	else if ( text == "quit" )
	{
		r.type = SceneRequest::Type::QUIT;
	}
	else if ( starts_with( "push:" ) )
	{
		r.type = SceneRequest::Type::PUSH_OVERLAY;
		split_scene_name( text.substr( 5 ), r.name, r.args );
	}
	else if ( starts_with( "base:" ) )
	{
		r.type = SceneRequest::Type::REPLACE_BASE;
		split_scene_name( text.substr( 5 ), r.name, r.args );
	}
	else
	{
		r.type = from_overlay ? SceneRequest::Type::REPLACE_OVERLAY : SceneRequest::Type::REPLACE_BASE;
		split_scene_name( text, r.name, r.args );
	}

	return r;
}

} // namespace blue_sky
