#pragma once

#include <type/type.h>
#include <unordered_map>

namespace game
{

/**
 * �����L���b�V��
 *
 */
template< typename T >
class TimedCache
{
public:
	using TimerMap = std::unordered_map< T, float_t >;

private:
	TimerMap timer_map_; ///< �ŋߐڐG�����I�u�W�F�N�g�̈ꗗ

public:
	TimedCache()
	{ }

	/**
	 * �X�V����
	 *
	 * @param elapsed �O�񂩂�̌o�ߎ��� ( �b )
	 * @param bool ������ cleanup() ���s���t���O
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
	 * ���ꂽ�I�u�W�F�N�g���ꗗ����폜����
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
	 * �L���b�V���ɒǉ�����
	 *
	 * @param o �I�u�W�F�N�g
	 * @param life_time �L���b�V������폜�����܂ł̎��� ( �b )
	 */
	void cache( T o, float_t life_time )
	{
		timer_map_[ o ] = life_time;
	}

	/**
	 * ���݃L���b�V������Ă��邩��Ԃ�
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
	 * �L���b�V�����N���A����
	 *
	 */
	void clear()
	{
		timer_map_.clear();
	}

}; // class TimedCache

} // namespace game
