#pragma once

#include <cstdint>

namespace core
{

/**
 * リソースを指すハンドル
 *
 * 一度使った番号は二度と使わないので、リソースが解放された後のハンドルは確実に「無効」と分かる
 * （生ポインタは解放後も同じ値のまま残り、使うと壊れる。設計上の問題 B20）。
 * 解放・差し替えがあり得るリソースを長く持つ側は、生ポインタではなくハンドルを持ち、使うたびに resolve する。
 * 中身の差し替え（ResourceManager::replace）では番号は変わらないので、持っているハンドルはそのまま新しい中身を指す。
 *
 * 引く側（ResourceManager）を含めずに持てるよう、ハンドルだけをこのヘッダに分けている（コンポーネントなどが持つ）。
 */
template< typename ResourceType >
struct ResourceHandle
{
	std::uint64_t id = 0;

	bool is_null() const { return id == 0; }
	bool operator == ( const ResourceHandle& o ) const { return id == o.id; }
	bool operator != ( const ResourceHandle& o ) const { return id != o.id; }
};

} // namespace core
