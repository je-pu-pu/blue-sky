#pragma once

#include <blue_sky/type.h>

namespace blue_sky
{

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

	bool operator == ( const SceneRequest& r ) const { return type == r.type && name == r.name && stage_name == r.stage_name; }
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
		r.name = text.substr( 5 );
	}
	else if ( starts_with( "base:" ) )
	{
		r.type = SceneRequest::Type::REPLACE_BASE;
		r.name = text.substr( 5 );
	}
	else
	{
		r.type = from_overlay ? SceneRequest::Type::REPLACE_OVERLAY : SceneRequest::Type::REPLACE_BASE;
		r.name = text;
	}

	return r;
}

} // namespace blue_sky
