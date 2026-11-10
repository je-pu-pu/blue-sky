#pragma once

#include "type.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <vector>

namespace core::ecs
{

/**
 * Entity の番号の順（= 作った順）に並べた一覧
 *
 * 作った順に回したい System（ObjectRenderDataSystem・MoveToTargetSystem）が使う。
 * 以前は対象が 1 つ増減するたびに一覧を全部作り直して並べ替えていて（O(n log n)）、
 * Entity が 1 万を超えると、物体が生まれる・消えるフレームが重くなった。
 *
 * - 足す : EntityId は二度と使わない増える一方の通し番号なので、たいていは末尾に足すだけで順が保てる（O(1)）。
 *          今までの最大より小さい番号（System を後から足した・部品を付け直した）だけ、二分探索して途中に入れる（O(n) の移動）
 * - 消す : 印を付けるだけ（二分探索で O(log n)）。次に一覧を読むとき（get）に、印の付いたものをまとめて詰める（O(n) を 1 回）
 * - 引く : 二分探索（O(log n)）
 *
 * T は EntityId そのものか、メンバ id（EntityId）を持つ型。
 * get が返す一覧・find が返すポインタは、次に足す・詰めるまで有効
 */
template< typename T >
class EntityIdOrderedList
{
public:
	using ItemList = std::vector< T >;

private:
	static constexpr std::size_t NOT_FOUND = static_cast< std::size_t >( -1 );

	mutable ItemList					item_list_;			///< 番号の小さい順（消した印の付いたものも、詰めるまで残る）
	mutable std::vector< std::uint8_t >	alive_list_;		///< item_list_ と同じ並びで、生きていれば 1
	mutable std::size_t					removed_count_ = 0;	///< 消した印の数（0 なら詰めなくてよい）

	static EntityId id_of( const T& item )
	{
		if constexpr ( std::is_same_v< T, EntityId > )
		{
			return item;
		}
		else
		{
			return item.id;
		}
	}

	/// id 以上の最初の添字
	std::size_t lower_index( EntityId id ) const
	{
		const auto i = std::lower_bound( item_list_.begin(), item_list_.end(), id, [] ( const T& item, EntityId id ) { return id_of( item ) < id; } );

		return static_cast< std::size_t >( i - item_list_.begin() );
	}

	/// id の添字（消した印の付いたものも返す。無ければ NOT_FOUND）
	std::size_t find_index( EntityId id ) const
	{
		const auto n = lower_index( id );

		return ( n < item_list_.size() && id_of( item_list_[ n ] ) == id ) ? n : NOT_FOUND;
	}

	/// 消した印の付いたものを詰める（順は保つ）
	void compact() const
	{
		if ( removed_count_ == 0 )
		{
			return;
		}

		std::size_t w = 0;

		for ( std::size_t r = 0; r < item_list_.size(); r++ )
		{
			if ( ! alive_list_[ r ] )
			{
				continue;
			}

			if ( w != r )
			{
				item_list_[ w ] = std::move( item_list_[ r ] );
			}

			w++;
		}

		item_list_.erase( item_list_.begin() + static_cast< std::ptrdiff_t >( w ), item_list_.end() );
		alive_list_.assign( w, 1 );
		removed_count_ = 0;
	}

public:
	/**
	 * 足す（同じ番号が既にあれば中身を置き換える。消した印が付いていれば生き返らせる）
	 *
	 * @return 足した（置き換えた）要素。次に足す・詰めるまで有効
	 */
	T& insert( const T& item )
	{
		const EntityId id = id_of( item );

		if ( item_list_.empty() || id_of( item_list_.back() ) < id )
		{
			item_list_.push_back( item );
			alive_list_.push_back( 1 );

			return item_list_.back();
		}

		const auto n = lower_index( id );

		if ( id_of( item_list_[ n ] ) == id )
		{
			item_list_[ n ] = item;

			if ( ! alive_list_[ n ] )
			{
				alive_list_[ n ] = 1;
				removed_count_--;
			}

			return item_list_[ n ];
		}

		item_list_.insert( item_list_.begin() + static_cast< std::ptrdiff_t >( n ), item );
		alive_list_.insert( alive_list_.begin() + static_cast< std::ptrdiff_t >( n ), 1 );

		return item_list_[ n ];
	}

	/**
	 * 消す（印を付けるだけ。詰めるのは次に get したとき）
	 *
	 * @return 生きていたものを消したら true
	 */
	bool erase( EntityId id )
	{
		const auto n = find_index( id );

		if ( n == NOT_FOUND || ! alive_list_[ n ] )
		{
			return false;
		}

		alive_list_[ n ] = 0;
		removed_count_++;

		return true;
	}

	/// id の要素（無い・消した後なら nullptr）
	T* find( EntityId id )
	{
		const auto n = find_index( id );

		return ( n != NOT_FOUND && alive_list_[ n ] ) ? & item_list_[ n ] : nullptr;
	}

	const T* find( EntityId id ) const
	{
		const auto n = find_index( id );

		return ( n != NOT_FOUND && alive_list_[ n ] ) ? & item_list_[ n ] : nullptr;
	}

	/// 番号の小さい順の一覧（消した印の付いたものがあれば、先に詰める）。中身は書き換えてよいが、番号は変えないこと
	ItemList& get()
	{
		compact();

		return item_list_;
	}

	const ItemList& get() const
	{
		compact();

		return item_list_;
	}

	/// 生きている要素の数
	std::size_t size() const { return item_list_.size() - removed_count_; }

	void clear()
	{
		item_list_.clear();
		alive_list_.clear();
		removed_count_ = 0;
	}

}; // class EntityIdOrderedList

} // namespace core::ecs
