#pragma once

#include <type/type.h>
#include <unordered_map>

namespace game
{

/**
 * 時限キャッシュ
 *
 */
template< typename T >
class TimedCache
{
public:
	using TimerMap = std::unordered_map< T, float_t >;

private:
	TimerMap timer_map_; ///< 最近接触したオブジェクトの一覧

public:
	TimedCache()
	{ }

	/**
	 * 更新する
	 *
	 * @param elapsed 前回からの経過時間 ( 秒 )
	 * @param bool 同時に cleanup() を行うフラグ
	 */
	void update( float_t elapsed, bool cleanup = true )
	{
		for ( auto i = timer_map_.begin(); i != timer_map_.end(); )
		{
			i->second -= elapsed;

			if ( cleanup && i->second <= 0.f )
			{
				i = timer_map_.erase( i );
			}
			else
			{
				++i;
			}
		}
	}

	/**
	 * 離れたオブジェクトを一覧から削除する
	 *
	 */
	void cleanup()
	{
		for ( auto i = timer_map_.begin(); i != timer_map_.end(); )
		{
			if ( i->second <= 0.f )
			{
				i = timer_map_.erase( i );
			}
			else
			{
				++i;
			}
		}
	}

	/**
	 * キャッシュに追加する
	 *
	 * @param o オブジェクト
	 * @param life_time キャッシュから削除されるまでの時間 ( 秒 )
	 */
	void cache( T o, float_t life_time )
	{
		timer_map_[ o ] = life_time;
	}

	/**
	 * 現在キャッシュされているかを返す
	 *
	 */
	bool_t is_cached( T o ) const
	{
		auto i = timer_map_.find( o );

		if ( i == timer_map_.end() )
		{
			return false;
		}

		return i->second > 0.f;
	}
	
	/**
	 * キャッシュをクリアする
	 *
	 */
	void clear()
	{
		timer_map_.clear();
	}

}; // class TimedCache

} // namespace game
