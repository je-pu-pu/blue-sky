#pragma once

#include <cstddef>
#include <functional>

namespace core
{

template< typename ResourceType > class ResourceManager;

/**
 * 持ち主の印（doc/ハンドルの設計.md）
 *
 * ResourceManager の置き場（ページ）にだけある。作るのも、中身を書き換えるのも ResourceManager だけ
 * （コンストラクタと中身は ResourceManager にだけ見せる。ほかの所では作れない : Handle< T > h; はコンパイルできない）。
 * コピーもムーブもできない（作ったら動かさない・解放しない。WeakHandle がこの場所を指すため）。
 *
 * 中身は実際のリソースへのポインタだけ（8 バイト）。リソースを解放したら nullptr、差し替えたら新しい中身を指す。
 */
template< typename ResourceType >
class Handle
{
	friend class ResourceManager< ResourceType >;

	ResourceType* resource_ = nullptr;	///< 実際のリソース。解放したら nullptr

	Handle() = default;

public:
	~Handle() = default;

	Handle( const Handle& ) = delete;
	Handle& operator = ( const Handle& ) = delete;
	Handle( Handle&& ) = delete;
	Handle& operator = ( Handle&& ) = delete;

	ResourceType* get() const { return resource_; }
};

/**
 * リソースを指すハンドル（持ち主ではない側が持つ。weak_ptr と同じ使い方。doc/ハンドルの設計.md）
 *
 * Handle（ResourceManager の置き場にある持ち主の印）へのポインタだけを持つ（8 バイト。コピーできる）。
 * get() は Handle をたどってリソースを返す。解放された後は nullptr（以前の生ポインタのように壊れたアドレスを指さない。設計上の問題 B20）。
 * 中身の差し替え（ResourceManager::replace）・同じ名前の読み直しでは同じ Handle を使うので、持っている WeakHandle はそのまま新しい中身を引く。
 *
 * 1 フレームの中では、get() で引いたポインタを使い回してよい（解放と差し替えはフレームの終わりにまとめている）。次のフレームへは持ち越さない。
 * ResourceManager（GraphicsManager・SoundManager が持つ）はアプリの間ずっと生きているので、Handle の置き場が先に壊れることはない
 * （WeakHandle を static・グローバルに置かない）。
 *
 * 引く側（ResourceManager）を含めずに持てるよう、このヘッダに分けている（コンポーネントなどが持つ）。
 */
template< typename ResourceType >
class WeakHandle
{
	friend class ResourceManager< ResourceType >;

	const Handle< ResourceType >* handle_ = nullptr;

	explicit WeakHandle( const Handle< ResourceType >* handle ) : handle_( handle ) { }

public:
	WeakHandle() = default;

	/// リソースを引く（空・解放済みなら nullptr）。weak_ptr::lock に当たる
	ResourceType* get() const { return handle_ ? handle_->get() : nullptr; }

	/// 引けない（空・解放済み）
	bool expired() const { return get() == nullptr; }

	/// 何も指していない（空のハンドル）。解放済みかどうかは expired で見る
	bool is_null() const { return handle_ == nullptr; }

	bool operator == ( const WeakHandle& o ) const { return handle_ == o.handle_; }
	bool operator != ( const WeakHandle& o ) const { return handle_ != o.handle_; }

	/// 表の鍵・並べ替えに使う値（指している Handle の場所。同じ Handle なら同じ）
	const void* key() const { return handle_; }
};

/// 以前の名前（ModelHandle・TextureHandle・ShaderHandle・SoundHandle はこの別名）
template< typename ResourceType >
using ResourceHandle = WeakHandle< ResourceType >;

} // namespace core

template< typename ResourceType >
struct std::hash< core::WeakHandle< ResourceType > >
{
	std::size_t operator () ( const core::WeakHandle< ResourceType >& h ) const noexcept { return std::hash< const void* >()( h.key() ); }
};
