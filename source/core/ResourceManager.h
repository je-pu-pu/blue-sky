#pragma once

#include <type/type.h>

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <memory>
#include <unordered_map>
#include <vector>

namespace core
{

/**
 * リソースを指すハンドル
 *
 * 一度使った番号は二度と使わないので、リソースが解放された後のハンドルは確実に「無効」と分かる
 * （生ポインタは解放後も同じ値のまま残り、使うと壊れる。設計上の問題 B20）。
 * 解放・差し替えがあり得るリソースを長く持つ側は、生ポインタではなくハンドルを持ち、使うたびに resolve する。
 */
template< typename ResourceType >
struct ResourceHandle
{
	std::uint64_t id = 0;

	bool is_null() const { return id == 0; }
	bool operator == ( const ResourceHandle& o ) const { return id == o.id; }
	bool operator != ( const ResourceHandle& o ) const { return id != o.id; }
};

/**
 * リソース管理テンプレートクラス
 *
 * このクラスは複数のリソースを所有し、名前を付けて検索できるようにする（スクリプトからのアクセスを容易にする）。
 * 全てのリソースはこのクラスのデストラクタ、または remove_named（明示的なアンロード）で解放される。
 *
 * - 同じ名前で登録し直すと、名前は新しいリソースを指す。古いリソースは解放しない
 *   （各所が生ポインタで持っているため。例 : シーンごとに setup_default_shaders が同じ名前でシェーダーを作り直し、
 *     古いシェーダーはモデルが使い続けている。解放すると落ちる）
 * - 名前を削除したとき、別の名前でも同じリソースを指していれば解放しない（以前は解放して、別の名前が無効なポインタを指した）
 * - 解放されたリソースのハンドルは resolve で nullptr になる。解放があり得るリソースを長く持つ側はハンドルを持つ
 */
template< typename ResourceType >
class ResourceManager
{
public:
	using Resource		= ResourceType;
	using ResourceList	= std::vector< std::unique_ptr< Resource > >;
	using ResourceMap	= std::unordered_map< string_t, Resource* >;
	using Handle		= ResourceHandle< ResourceType >;

private:
	ResourceList resource_list_;
	ResourceMap resource_map_;

	std::unordered_map< const Resource*, std::uint64_t > id_of_;	///< リソース → ハンドルの番号
	std::unordered_map< std::uint64_t, Resource* > by_id_;			///< ハンドルの番号 → リソース
	std::uint64_t next_id_ = 1;

	/// 所有しているか
	bool owns( const Resource* resource ) const { return id_of_.find( resource ) != id_of_.end(); }

	/// どれかの名前が指しているか
	bool is_named( const Resource* resource ) const
	{
		return std::any_of( resource_map_.begin(), resource_map_.end(), [resource] ( const auto& x ) { return x.second == resource; } );
	}

	/// 解放する（ハンドルは無効になる）
	void destroy( Resource* resource )
	{
		auto id = id_of_.find( resource );

		if ( id == id_of_.end() )
		{
			return;
		}

		by_id_.erase( id->second );
		id_of_.erase( id );

		auto i = std::find_if( resource_list_.begin(), resource_list_.end(), [resource] ( const auto& x ) { return x.get() == resource; } );

		if ( i != resource_list_.end() )
		{
			resource_list_.erase( i );
		}
	}

public:
	ResourceManager() = default;
	virtual ~ResourceManager() = default;

	ResourceManager( const ResourceManager& ) = delete;
	ResourceManager& operator = ( const ResourceManager& ) = delete;

	/**
	 * 指定した型のリソースを作成する
	 */
	template< typename Type = Resource, typename ... Args >
	Type* create( Args ... args )
	{
		Type* resource = new Type( args ... );
		add( resource );

		return resource;
	}

	/**
	 * 指定した型のリソースを作成し名前を付ける（同じ名前のリソースがあれば解放して差し替える）
	 *
	 * @param name 作成したリソースに付ける名前
	 */
	template< typename Type = Resource, typename ... Args >
	Type* create_named( const string_t& name, Args ... args )
	{
		Type* resource = new Type( args ... );
		add_named( name, resource );

		return resource;
	}

	/**
	 * 外部で作成したリソースを追加し ResourceManager で管理する（nullptr は無視する）
	 *
	 * @param resource リソース
	 */
	void add( ResourceType* resource )
	{
		if ( ! resource || owns( resource ) )
		{
			return;
		}

		resource_list_.emplace_back( resource );

		const std::uint64_t id = next_id_++;
		id_of_[ resource ] = id;
		by_id_[ id ] = resource;
	}

	/**
	 * 外部で作成したリソースを名前を付けて追加し ResourceManager で管理する
	 *
	 * 同じ名前で別のリソースが登録されていれば、名前だけを付け替える（古いリソースは所有したまま。生ポインタで使われ得るため）。
	 *
	 * @param name リソースに付ける名前
	 * @param resource リソース（nullptr なら何もしない）
	 */
	void add_named( const string_t& name, ResourceType* resource )
	{
		if ( ! resource )
		{
			return;
		}

		add( resource );

		resource_map_[ name ] = resource;
	}

	/**
	 * リソースを解放する（それを指す名前も外す。ハンドルは無効になる）
	 *
	 * 作った側が、使い終わったリソースを手放すときに使う（他から生ポインタで使われていないことは呼ぶ側が保証する）
	 */
	void release( Resource* resource )
	{
		for ( auto i = resource_map_.begin(); i != resource_map_.end(); )
		{
			i = i->second == resource ? resource_map_.erase( i ) : std::next( i );
		}

		destroy( resource );
	}

	/**
	 * 名前を削除する。どの名前からも指されなくなったリソースは解放する
	 */
	void remove_named( const string_t& name )
	{
		auto i = resource_map_.find( name );

		if ( i == resource_map_.end() )
		{
			return;
		}

		Resource* resource = i->second;
		resource_map_.erase( i );

		if ( ! is_named( resource ) )
		{
			destroy( resource );
		}
	}

	/**
	 * 名前でリソースを検索し取得する
	 *
	 * @param name リソースに付けた名前
	 * @return リソースが見つかった場合はリソースを、見つからなかった場合は nullptr を返す
	 */
	template< typename Type = ResourceType >
	Type* get( const string_t& name )
	{
		auto i = resource_map_.find( name );

		if ( i == resource_map_.end() )
		{
			return nullptr;
		}

		return static_cast< Type* >( i->second );
	}

	/// 名前のリソースのハンドル（無ければ空のハンドル）
	Handle get_handle( const string_t& name ) const
	{
		auto i = resource_map_.find( name );
		return i == resource_map_.end() ? Handle() : handle_of( i->second );
	}

	/// リソースのハンドル（所有していなければ空のハンドル）
	Handle handle_of( const Resource* resource ) const
	{
		auto i = id_of_.find( resource );
		return i == id_of_.end() ? Handle() : Handle{ i->second };
	}

	/// ハンドルからリソースを得る（解放済み・空なら nullptr）
	Resource* resolve( Handle handle ) const
	{
		auto i = by_id_.find( handle.id );
		return i == by_id_.end() ? nullptr : i->second;
	}

	/**
	 * リソースの一覧を取得する
	 */
	const ResourceList& get_resource_list() const
	{
		return resource_list_;
	}

	/**
	 * 全てのリソースを削除する（全てのハンドルは無効になる）
	 */
	void clear_all()
	{
		resource_list_.clear();
		resource_map_.clear();
		id_of_.clear();
		by_id_.clear();
	}

	/**
	 * デバッグのためにリソースの一覧を標準出力に出力する
	 */
	void debug_print_resources() const
	{
		std::cout << "list : " << resource_list_.size() << ", named : " << resource_map_.size() << '\n';

		for ( const auto& r : resource_map_ )
		{
			std::cout << "\t" << r.first << " : " << r.second << '\n';
		}

		std::cout << std::flush;
	}
};

} // namespace core
