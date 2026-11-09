#pragma once

#include <core/type.h>

namespace core::setting
{

/**
 * 型付きの設定キー
 *
 * 設定の名前と値の型を組にして持つ。SettingStore::get( key ) / set( key, value ) に渡すと、
 * 型の書き間違い（float の項目を int で読むなど）がコンパイル時に分かる。
 *
 * @code
 * inline constexpr SettingKey< float_t > AUDIO_VOLUME { "audio.volume" };
 * store.set( AUDIO_VOLUME, 0.5f );
 * const float_t v = store.get( AUDIO_VOLUME );
 * @endcode
 *
 * 文字列（名前）へ暗黙に変換できるので、名前で扱う既存の API にもそのまま渡せる。
 */
template< typename T >
struct SettingKey
{
	using ValueType = T;

	const char_t* name;

	constexpr operator const char_t* () const { return name; }
	operator string_t () const { return name; }
	string_t str() const { return name; }
};

} // namespace core::setting
