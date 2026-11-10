#pragma once

#include "type.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace core::ecs
{

/**
 * EntityId から添字（uint32_t）を引く表
 *
 * EntityId は二度と使わない通し番号で、生きている番号は「最近作ったもの」の範囲に固まる。
 * 番号を PAGE_SIZE 個ごとのページに分け、ページの中は配列で直に引く（ハッシュを計算せず、表引きは 2 回の読み出し）。
 * ページの中の番号が全部消えたらページを放すので、番号が増え続けても、使う量は生きている番号の範囲で決まる
 * （残るのはページへのポインタの一覧だけで、番号 1024 個につき 8 バイト）。
 *
 * unordered_map< EntityId, uint32_t > と比べた結果は doc/ECS移行計画.md の 6 節（EcsBenchmark.DISABLED_SparseIndexVsUnorderedMap）
 */
class SparseIndex
{
public:
	using Index = std::uint32_t;

	static constexpr Index INVALID = ~Index( 0 );			///< 登録されていない

	static constexpr std::size_t PAGE_BITS = 10;
	static constexpr std::size_t PAGE_SIZE = std::size_t( 1 ) << PAGE_BITS;	///< 1 ページの番号の数（1 ページ 4 KB）

private:
	struct Page
	{
		Index index[ PAGE_SIZE ];
		std::size_t count = 0;		///< このページで登録されている番号の数（0 になったら放す）

		Page() { std::fill( std::begin( index ), std::end( index ), INVALID ); }
	};

	std::vector< std::unique_ptr< Page > > pages_;
	std::size_t size_ = 0;

	static std::size_t page_of( EntityId id ) { return static_cast< std::size_t >( id ) >> PAGE_BITS; }
	static std::size_t offset_of( EntityId id ) { return static_cast< std::size_t >( id ) & ( PAGE_SIZE - 1 ); }

public:
	SparseIndex() = default;
	SparseIndex( SparseIndex&& ) noexcept = default;
	SparseIndex& operator = ( SparseIndex&& ) noexcept = default;

	/// id の添字（無ければ INVALID）
	Index find( EntityId id ) const
	{
		const auto p = page_of( id );

		if ( p >= pages_.size() || ! pages_[ p ] )
		{
			return INVALID;
		}

		return pages_[ p ]->index[ offset_of( id ) ];
	}

	bool contains( EntityId id ) const { return find( id ) != INVALID; }

	/// id の添字を設定する（既にあれば置き換える）。index に INVALID は渡さない（消すのは erase）
	void set( EntityId id, Index index )
	{
		const auto p = page_of( id );

		if ( p >= pages_.size() )
		{
			pages_.resize( p + 1 );
		}

		if ( ! pages_[ p ] )
		{
			pages_[ p ] = std::make_unique< Page >();
		}

		auto& page = * pages_[ p ];
		auto& slot = page.index[ offset_of( id ) ];

		if ( slot == INVALID )
		{
			page.count++;
			size_++;
		}

		slot = index;
	}

	/// id を消す（無ければ何もせず false）。ページが空になったら放す
	bool erase( EntityId id )
	{
		const auto p = page_of( id );

		if ( p >= pages_.size() || ! pages_[ p ] )
		{
			return false;
		}

		auto& page = * pages_[ p ];
		auto& slot = page.index[ offset_of( id ) ];

		if ( slot == INVALID )
		{
			return false;
		}

		slot = INVALID;
		size_--;

		if ( --page.count == 0 )
		{
			pages_[ p ].reset();
		}

		return true;
	}

	/// 登録されている番号の数
	std::size_t size() const { return size_; }

	bool empty() const { return size_ == 0; }

	void clear()
	{
		pages_.clear();
		size_ = 0;
	}

	/// 確保しているページの数（テスト・デバッグ用）
	std::size_t debug_page_count() const
	{
		return static_cast< std::size_t >( std::count_if( pages_.begin(), pages_.end(), [] ( const auto& p ) { return p != nullptr; } ) );
	}

}; // class SparseIndex

} // namespace core::ecs
