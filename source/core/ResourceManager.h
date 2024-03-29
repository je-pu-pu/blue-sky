#pragma once

#include <type/type.h>
#include <unordered_map>
#include <execution>

#include <iostream>

namespace core
{

/**
 * リソース管理テンプレートクラス
 *
 * このクラスは複数のリソースを管理する
 *
 * 全てのリソースはこのクラスのデストラクタによりメモリ解放される
 *
 * このクラスはリソースに名前を付けて管理し、名前でリソースを検索し取得できる
 * ( これはスクリプトからのアクセスを容易にする )
 */
template< typename ResourceType >
class ResourceManager
{
public:
	using Resource		= ResourceType;
	using ResourceList	= std::vector< std::unique_ptr< Resource > >;
	using ResourceMap	= std::unordered_map< string_t, Resource* >;

private:
	ResourceList resource_list_;
	ResourceMap resource_map_;

public:
	ResourceManager()
	{

	}

	virtual ~ResourceManager()
	{

	}

	/**
	 * 指定した型のリソースを作成する
	 *
	 */
	template< typename Type = Resource, typename ... Args >
	Type* create( Args ... args )
	{
		Type* resource = new Type( args ... );
		resource_list_.emplace_back( resource );

		return resource;
	}

	/**
	 * 指定した型のリソースを作成し名前を付ける
	 *
	 * @param name 作成したリソースに付ける名前
	 */
	template< typename Type = Resource, typename ... Args >
	Type* create_named( const string_t& name, Args ... args )
	{
		Type* resource = create< Type >( args ... );
		resource_map_[ name ] = resource;

		return resource;
	}

	/**
	 * 外部で作成したリソースを追加し ResourceManager で管理する
	 *
	 * @param resource リソース
	 */
	void add( ResourceType* resource )
	{
		resource_list_.emplace_back( resource );
	}

	/**
	 * 外部で作成したリソースを名前を付けて追加し ResourceManager で管理する
	 *
	 * @param name リソースに付ける名前
	 * @param resource リソース
	 */
	void add_named( const string_t& name, ResourceType* resource )
	{
		add( resource );
		resource_map_[ name ] = resource;
	}

	/**
	 * add_named() で追加したリソースを削除する
	 */
	void remove_named( const string_t& name )
	{
		auto i = resource_map_.find( name );

		if ( i == resource_map_.end() )
		{
			return;
		}

		resource_list_.erase( std::find_if( std::execution::par_unseq, resource_list_.begin(), resource_list_.end(), [&] ( const auto& x ) { return x.get() == i->second; } ) );
		resource_map_.erase( i );
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

	/**
	 * リソースの一覧を取得する
	 */
	const ResourceList& get_resource_list() const
	{
		return resource_list_;
	}

	/**
	 * 全てのリソースを削除する
	 *
	 */
	bool_t clear_all()
	{
		resource_list_.clear();
		resource_map_.clear();
	}

	/**
	 * デバッグのためにリソースの一覧を標準出力に出力する
	 *
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
