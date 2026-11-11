#pragma once

#include <core/ResourceHandle.h>
#include <type/type.h>

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>

namespace core
{

/**
 * リソース管理テンプレートクラス
 *
 * このクラスは複数のリソースを所有し、名前を付けて検索できるようにする（スクリプトからのアクセスを容易にする）。
 * 全てのリソースはこのクラスのデストラクタ、または remove_named（明示的なアンロード）で解放される。
 *
 * ハンドル（doc/ハンドルの設計.md）:
 * - 持ち主の印 Handle は、このクラスだけが作り、置き場（ページ）に置く。作ったら動かさない・解放しない
 *   （ページは足すだけで、既存のページは動かさない・捨てない。core/ecs/StablePool.h と同じ考え方）
 * - ほかの所は WeakHandle（Handle へのポインタ、8 バイト）を持ち、get() で引く。引くのはポインタを 2 回たどるだけ
 *   （以前は番号からハッシュの表を引いていた）
 * - 解放すると Handle の中のポインタを nullptr にする。差し替えは Handle の中のポインタを替えるだけ
 * - 一度解放した名前をもう一度読んだら、同じ Handle を使い回す（古い WeakHandle は再び引ける。名前で読むリソースは Handle が増えない）。
 *   名前の無いリソース（clone_model など）の Handle は使い回さない（古い WeakHandle が別のものを指さないように）ので、作るたびに増える
 * - 書き換えるのはメインスレッドだけ（別のスレッドで読んだ物はメインスレッドで登録する）
 *
 * 名前:
 * - 同じ名前で登録し直すと、名前は新しいリソース（新しい Handle）を指す。古いリソースは解放せず、古いハンドルも古いリソースを引く
 *   （例 : シーンごとに setup_default_shaders が同じ名前でシェーダーを作り直し、古いシェーダーはモデルが使い続けている）
 * - 名前を削除したとき、別の名前でも同じリソースを指していれば解放しない
 * - replace は同じ Handle のまま中身を差し替える（名前も同じ Handle を指すので、新しい中身を引く）
 *
 * 解放の時期:
 * - 解放（release・remove_named）と差し替え（replace）で手放す古いリソースは、すぐには解放せず、
 *   フレームの終わり（Present の後）とシーンの切り替えで呼ぶ flush_releases でまとめて解放する。
 *   1 フレームの中では、引いたポインタを使い回してよい（次のフレームへは持ち越さない）。
 *   解放を頼んだリソースのハンドルは、flush_releases までは引ける（差し替えたハンドルは、すぐに新しい中身を引く）
 *
 * - 基底の型で管理し、派生の型のリソースを入れてもよい（例 : core::graphics::Model で管理し blue_sky のモデルを入れる）。
 *   基底の型のデストラクタは virtual にすること
 */
template< typename ResourceType >
class ResourceManager
{
public:
	using Resource		= ResourceType;
	using ResourceList	= std::vector< std::unique_ptr< Resource > >;
	using OwnerHandle	= core::Handle< ResourceType >;			///< 持ち主の印（このクラスの中だけ）
	using WeakHandle	= core::WeakHandle< ResourceType >;		///< ほかの所が持つハンドル

	static constexpr std::size_t HANDLE_PAGE_SIZE = 256;		///< 1 ページの Handle の数（8 バイト × 256 = 2 KB）

private:
	/// Handle の置き場。ページを足すだけで、既存のページは動かさない・捨てない（WeakHandle が場所を指すため）
	/// 先に宣言する（リソースより後に壊す。リソースのデストラクタの間も Handle は生きている）
	std::vector< std::unique_ptr< OwnerHandle[] > > handle_pages_;
	std::size_t handle_count_ = 0;

	ResourceList resource_list_;													///< 所有しているリソース
	std::unordered_map< string_t, OwnerHandle* > resource_map_;					///< 名前 → Handle
	std::unordered_map< const Resource*, OwnerHandle* > handle_of_;				///< 所有しているリソース → Handle（ハンドルを渡すとき・所有しているかを見るときだけ引く）
	std::unordered_map< string_t, OwnerHandle* > released_names_;				///< 解放した名前 → その Handle（同じ名前を読み直したら使い回す）

	/// 解放を頼まれたリソース（フレームの終わりの flush_releases まで生かしておく。ハンドルもその時まで引ける）
	struct PendingRelease
	{
		std::unique_ptr< Resource >	resource;
		OwnerHandle*				handle;
	};

	std::vector< PendingRelease > pending_releases_;

	/// Handle を 1 つ置き場に作る（ページが一杯ならページを足す）
	OwnerHandle* new_handle()
	{
		if ( handle_count_ % HANDLE_PAGE_SIZE == 0 )
		{
			handle_pages_.emplace_back( new OwnerHandle[ HANDLE_PAGE_SIZE ] );	// Handle のコンストラクタはこのクラスにだけ見える
		}

		return & handle_pages_.back()[ handle_count_++ % HANDLE_PAGE_SIZE ];
	}

	/// 所有しているリソースの Handle（所有していなければ nullptr）
	OwnerHandle* find_handle( const Resource* resource ) const
	{
		auto i = handle_of_.find( resource );
		return i == handle_of_.end() ? nullptr : i->second;
	}

	/// 所有しているか
	bool owns( const Resource* resource ) const { return resource && handle_of_.find( resource ) != handle_of_.end(); }

	/// どれかの名前が Handle を指しているか
	bool is_named( const OwnerHandle* handle ) const
	{
		return std::any_of( resource_map_.begin(), resource_map_.end(), [handle] ( const auto& x ) { return x.second == handle; } );
	}

	/// 所有を Handle に結び付けて一覧に入れる
	void own( Resource* resource, OwnerHandle* handle )
	{
		resource_list_.emplace_back( resource );
		handle_of_[ resource ] = handle;
		handle->resource_ = resource;
	}

	/// 所有の一覧から外して解放待ちに移す（Handle はフレームの終わりまで今のリソースを指す）
	void move_to_pending( Resource* resource, OwnerHandle* handle )
	{
		handle_of_.erase( resource );

		auto i = std::find_if( resource_list_.begin(), resource_list_.end(), [resource] ( const auto& x ) { return x.get() == resource; } );

		if ( i != resource_list_.end() )
		{
			pending_releases_.push_back( { std::move( *i ), handle } );
			resource_list_.erase( i );
		}
	}

	/// 解放を頼む（Handle が nullptr になるのはフレームの終わりの flush_releases）
	void destroy( Resource* resource )
	{
		if ( OwnerHandle* handle = find_handle( resource ) )
		{
			move_to_pending( resource, handle );
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
	 * 指定した型のリソースを作成し名前を付ける
	 *
	 * 同じ名前のリソースがあれば、名前だけを新しいリソースへ付け替える。古いリソースは解放せず所有したまま残る
	 * （生ポインタで使われ得るため。手放すときは作った側が release() を呼ぶ）。add_named と同じ。
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
	 * 外部で作成したリソースを追加し ResourceManager で管理する（nullptr・管理済みは無視する）
	 *
	 * 名前を付けないリソースには、毎回新しい Handle を作る
	 *
	 * @param resource リソース
	 */
	void add( ResourceType* resource )
	{
		if ( ! resource || owns( resource ) )
		{
			return;
		}

		own( resource, new_handle() );
	}

	/**
	 * 外部で作成したリソースを名前を付けて追加し ResourceManager で管理する
	 *
	 * - 同じ名前で別のリソースが登録されていれば、名前だけを新しいリソース（新しい Handle）へ付け替える
	 *   （古いリソースは所有したまま。生ポインタ・古いハンドルで使われ得るため）
	 * - 一度解放した名前なら、その名前の Handle を使い回す（古い WeakHandle は新しいリソースを引く）
	 * - すでに管理しているリソースなら、名前をその Handle に付ける
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

		OwnerHandle* handle = find_handle( resource );

		if ( ! handle )
		{
			auto released = released_names_.find( name );

			// 解放した名前の Handle を使い回す。ただし、その Handle がほかの名前で読み直した生きているリソースを指していたら使わない
			if ( resource_map_.find( name ) == resource_map_.end() && released != released_names_.end() && ! owns( released->second->resource_ ) )
			{
				handle = released->second;
			}
			else
			{
				handle = new_handle();
			}

			own( resource, handle );	// 解放待ちの古いリソースを指していても、ここで新しいリソースを指す（古いものはフレームの終わりに解放する）
		}

		released_names_.erase( name );
		resource_map_[ name ] = handle;
	}

	/**
	 * リソースを解放する（それを指す名前はすぐに外す。解放とハンドルが無効になるのはフレームの終わりの flush_releases）
	 *
	 * 作った側が、使い終わったリソースを手放すときに使う（他から生ポインタで使われていないことは呼ぶ側が保証する）
	 */
	void release( Resource* resource )
	{
		OwnerHandle* handle = find_handle( resource );

		if ( ! handle )
		{
			return;
		}

		for ( auto i = resource_map_.begin(); i != resource_map_.end(); )
		{
			if ( i->second == handle )
			{
				released_names_[ i->first ] = handle;
				i = resource_map_.erase( i );
			}
			else
			{
				++i;
			}
		}

		move_to_pending( resource, handle );
	}

	/**
	 * 名前を削除する。どの名前からも指されなくなったリソースは解放する（フレームの終わりの flush_releases で）
	 */
	void remove_named( const string_t& name )
	{
		auto i = resource_map_.find( name );

		if ( i == resource_map_.end() )
		{
			return;
		}

		OwnerHandle* handle = i->second;
		resource_map_.erase( i );

		if ( ! is_named( handle ) )
		{
			released_names_[ name ] = handle;
			destroy( handle->resource_ );
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

		return static_cast< Type* >( i->second->resource_ );
	}

	/// 名前のリソースのハンドル（無ければ空のハンドル）
	WeakHandle get_handle( const string_t& name ) const
	{
		auto i = resource_map_.find( name );
		return i == resource_map_.end() ? WeakHandle() : WeakHandle( i->second );
	}

	/// リソースのハンドル（所有していなければ空のハンドル）
	WeakHandle handle_of( const Resource* resource ) const
	{
		return WeakHandle( find_handle( resource ) );
	}

	/// ハンドルからリソースを得る（解放済み・空なら nullptr）。handle.get() と同じ（以前の呼び方を残している）
	Resource* resolve( WeakHandle handle ) const
	{
		return handle.get();
	}

	/**
	 * 同じ Handle のまま中身を差し替え、古い中身を解放する（解放はフレームの終わり）
	 *
	 * 同じハンドルを持つ側・名前は、すぐに新しい中身を引く。
	 * 古い中身の生ポインタは、フレームの終わり（flush_releases）で無効になる（差し替えたいものはハンドルで持つ）。
	 *
	 * @param handle 差し替えるハンドル
	 * @param resource 新しい中身（所有を引き取る）
	 * @return 差し替えたら true。ハンドルが無効（空・解放済み・解放待ち・ほかの ResourceManager のもの）・新しい中身が nullptr・
	 *         すでに管理しているリソースなら false
	 *         （false のとき、新しい中身は、すでに管理しているものでなければ解放する）
	 */
	bool replace( WeakHandle handle, std::unique_ptr< Resource > resource )
	{
		Resource* old_resource = handle.get();
		OwnerHandle* owner = old_resource ? find_handle( old_resource ) : nullptr;

		if ( ! owner || owner != handle.handle_ || ! resource )
		{
			return false;
		}

		if ( owns( resource.get() ) )
		{
			// 別のハンドルの中身（または今の中身）を渡された。二重に持つと二重に解放するので引き取らない
			resource.release();
			return false;
		}

		move_to_pending( old_resource, owner );		// 古い中身はフレームの終わりに解放する
		own( resource.release(), owner );			// Handle はすぐに新しい中身を指す

		return true;
	}

	/**
	 * 解放を頼まれたリソースをまとめて解放する（フレームの終わり（Present の後）とシーンの切り替えで呼ぶ）
	 *
	 * @return 解放した数
	 */
	std::size_t flush_releases()
	{
		const std::size_t count = pending_releases_.size();

		for ( const auto& pending : pending_releases_ )
		{
			// 差し替えた・同じ名前で読み直した Handle は新しい中身を指しているので、そのまま
			if ( pending.handle->resource_ == pending.resource.get() )
			{
				pending.handle->resource_ = nullptr;
			}
		}

		pending_releases_.clear();

		return count;
	}

	/// 解放を頼まれていて、まだ解放していない数
	std::size_t get_pending_release_count() const { return pending_releases_.size(); }

	/// 作った Handle の数（解放しても減らない。名前の無いリソースを作るたびに増える。doc/ハンドルの設計.md の 2 章）
	std::size_t get_handle_count() const { return handle_count_; }

	/// リソースを指している Handle の数（デバッグ表示用。Handle を全部見る）
	std::size_t get_live_handle_count() const
	{
		std::size_t count = 0;

		for ( std::size_t n = 0; n < handle_count_; n++ )
		{
			if ( handle_pages_[ n / HANDLE_PAGE_SIZE ][ n % HANDLE_PAGE_SIZE ].resource_ )
			{
				count++;
			}
		}

		return count;
	}

	/// Handle の置き場の大きさ（バイト。ページの数 × ページの大きさ）
	std::size_t get_handle_memory_size() const { return handle_pages_.size() * HANDLE_PAGE_SIZE * sizeof( OwnerHandle ); }

	/**
	 * リソースの一覧を取得する
	 */
	const ResourceList& get_resource_list() const
	{
		return resource_list_;
	}

	/**
	 * 全てのリソースをすぐに削除する（全てのハンドルは無効になる。終わりの片付け用）
	 *
	 * Handle は残す（WeakHandle が指しているため）。名前は、読み直したときに同じ Handle を使い回せるよう覚えておく
	 */
	void clear_all()
	{
		for ( const auto& named : resource_map_ )
		{
			released_names_[ named.first ] = named.second;
		}

		for ( std::size_t n = 0; n < handle_count_; n++ )
		{
			handle_pages_[ n / HANDLE_PAGE_SIZE ][ n % HANDLE_PAGE_SIZE ].resource_ = nullptr;
		}

		pending_releases_.clear();
		resource_list_.clear();
		resource_map_.clear();
		handle_of_.clear();
	}

	/**
	 * デバッグのためにリソースの一覧を標準出力に出力する
	 */
	void debug_print_resources() const
	{
		std::cout << "list : " << resource_list_.size() << ", named : " << resource_map_.size() << ", handles : " << handle_count_ << '\n';

		for ( const auto& r : resource_map_ )
		{
			std::cout << "\t" << r.first << " : " << r.second->resource_ << '\n';
		}

		std::cout << std::flush;
	}
};

} // namespace core
