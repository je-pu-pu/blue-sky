#pragma once

#include "SparseIndex.h"
#include "type.h"

#include <cstddef>
#include <limits>
#include <memory>
#include <new>
#include <utility>
#include <vector>

namespace core::ecs
{

/**
 * 1 つの型の値を、Entity ごとに 1 つずつ持つ置き場（部品の型ごと・Entity の一覧に使う）
 *
 * 値はページ（1 ページ約 16 KB の連続した領域）に詰めて置き、1 つずつ new しない。
 * EntityId からの引き方は SparseIndex（ハッシュを使わない表）。
 *
 * **置いた値の場所は、消すまで動かない**（ポインタが安定する）。ページは伸びるときも動かさず、
 * 消したときも末尾と入れ替えない（空いた場所は次に置くときに使い回す）。
 * 部品のポインタを持ち続ける所（物理の剛体の userPointer の RigidBodyComponent*・GraphicsManager のカメラの部品・
 * シーンのメンバ・System の対象の一覧）が今も多くあり、場所が動く置き方では、どれかを直し漏らすと
 * ほかの Entity を消したときにだけ壊れる（見つけにくい）ため。
 * 代わりに、消した後に空きができると、置き場をそのまま回すときは空きを飛ばす。
 * System が回すのは置き場ではなく自分の対象の一覧（密な配列）なので、空きは走査の速さにほとんど効かない。
 */
template< typename T >
class StablePool
{
public:
	using Index = SparseIndex::Index;

	static constexpr std::size_t PAGE_BYTES = 16 * 1024;
	static constexpr std::size_t PAGE_CAPACITY = sizeof( T ) >= PAGE_BYTES ? 1 : PAGE_BYTES / sizeof( T );	///< 1 ページに置く数

private:
	static constexpr EntityId FREE = ( std::numeric_limits< EntityId >::max )();	///< 空いている場所の持ち主

	struct Page
	{
		alignas( T ) unsigned char storage[ sizeof( T ) * PAGE_CAPACITY ];
	};

	std::vector< std::unique_ptr< Page > >	pages_;			///< 値を置く領域（伸ばしても既にあるページは動かない）
	std::vector< EntityId >					slot_owner_;	///< 場所 → 持ち主の EntityId（空きは FREE）
	std::vector< Index >					free_slots_;	///< 空いている場所（後に空いたものから使う）
	SparseIndex								index_;			///< EntityId → 場所

	T* slot_pointer( Index slot )
	{
		return std::launder( reinterpret_cast< T* >( pages_[ slot / PAGE_CAPACITY ]->storage ) + slot % PAGE_CAPACITY );
	}

	const T* slot_pointer( Index slot ) const
	{
		return std::launder( reinterpret_cast< const T* >( pages_[ slot / PAGE_CAPACITY ]->storage ) + slot % PAGE_CAPACITY );
	}

	/// 空いている場所を 1 つ得る（無ければページを足す）
	Index acquire_slot()
	{
		if ( ! free_slots_.empty() )
		{
			const Index slot = free_slots_.back();
			free_slots_.pop_back();
			return slot;
		}

		const Index slot = static_cast< Index >( slot_owner_.size() );

		if ( slot % PAGE_CAPACITY == 0 )
		{
			pages_.push_back( std::make_unique< Page >() );
		}

		slot_owner_.push_back( FREE );

		return slot;
	}

public:
	StablePool() = default;
	~StablePool() { clear(); }

	StablePool( const StablePool& ) = delete;
	StablePool& operator = ( const StablePool& ) = delete;

	/**
	 * id の値を作って置く
	 *
	 * @return 置いた値（id の値が既にあれば何もせず nullptr）。消すまで同じ場所にある
	 */
	template< typename ... Args >
	T* emplace( EntityId id, Args&& ... args )
	{
		if ( index_.contains( id ) )
		{
			return nullptr;
		}

		const Index slot = acquire_slot();
		T* p = nullptr;

		try
		{
			p = new ( slot_pointer( slot ) ) T( std::forward< Args >( args ) ... );
		}
		catch ( ... )
		{
			free_slots_.push_back( slot );
			throw;
		}

		slot_owner_[ slot ] = id;
		index_.set( id, slot );

		return p;
	}

	/// id の値（無ければ nullptr）
	T* find( EntityId id )
	{
		const Index slot = index_.find( id );
		return slot == SparseIndex::INVALID ? nullptr : slot_pointer( slot );
	}

	const T* find( EntityId id ) const
	{
		const Index slot = index_.find( id );
		return slot == SparseIndex::INVALID ? nullptr : slot_pointer( slot );
	}

	bool contains( EntityId id ) const { return index_.contains( id ); }

	/// id の値を壊す（無ければ何もせず false）。ほかの値の場所は変わらない
	bool erase( EntityId id )
	{
		const Index slot = index_.find( id );

		if ( slot == SparseIndex::INVALID )
		{
			return false;
		}

		// 先に表から外す（壊している間に同じ id が引かれても、壊しかけの値を返さない）
		index_.erase( id );
		slot_owner_[ slot ] = FREE;

		std::destroy_at( slot_pointer( slot ) );
		free_slots_.push_back( slot );

		return true;
	}

	/// 全部の値を壊し、領域も放す
	void clear()
	{
		for ( std::size_t slot = 0; slot < slot_owner_.size(); slot++ )
		{
			if ( slot_owner_[ slot ] != FREE )
			{
				slot_owner_[ slot ] = FREE;
				std::destroy_at( slot_pointer( static_cast< Index >( slot ) ) );
			}
		}

		index_.clear();
		slot_owner_.clear();
		free_slots_.clear();
		pages_.clear();
	}

	/// 置いている値の数
	std::size_t size() const { return index_.size(); }

	bool empty() const { return index_.empty(); }

	/**
	 * 置いている値を、場所の順に回す（f( EntityId, T& )）
	 *
	 * 回している間に値を足す・消すのは不可（足した値を回すかが決まらない）
	 */
	template< typename F >
	void for_each( F&& f )
	{
		for ( std::size_t slot = 0; slot < slot_owner_.size(); slot++ )
		{
			if ( slot_owner_[ slot ] != FREE )
			{
				f( slot_owner_[ slot ], * slot_pointer( static_cast< Index >( slot ) ) );
			}
		}
	}

	/// 場所の数（空きを含む。テスト・デバッグ用）
	std::size_t debug_slot_count() const { return slot_owner_.size(); }

}; // class StablePool

} // namespace core::ecs
