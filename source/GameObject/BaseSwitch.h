#pragma once

#include "ActiveObject.h"
#include <game/TimedCache.h>
#include <list>

namespace blue_sky
{

/**
 * スイッチ基底クラス
 *
 */
class BaseSwitch : public ActiveObject
{
public:
	enum class State { OFF = 0, ON, BROKEN };

	using EventHandlerList	= std::list< EventHandler >;
	using EventHandlerMap	= std::unordered_map< string_t, EventHandlerList >;

	using CntactObjectCache	= game::TimedCache< const GameObject* >;
private:
	State state_;
	CntactObjectCache contact_object_cache_; ///< 最近スイッチに接触したオブジェクトのキャッシュ

	EventHandlerMap event_handler_map_;

	void on_collide_with( GameObject* o ) override { o->on_collide_with( this ); }
	void on_collide_with( Player* ) override;
	void on_collide_with( Stone* ) override;
	
	bool_t can_game_object_switch( const GameObject* ) const;

	virtual void on_turn_on() { }
	virtual void on_turn_off() { }
	virtual void on_break() { }

	void on_event( const string_t& name ) const
	{
		auto i = event_handler_map_.find( name );

		if ( i == event_handler_map_.end() )
		{
			return;
		}

		for ( auto j = i->second.begin(); j != i->second.end(); ++j )
		{
			( *j )();
		}
	}

public:
	BaseSwitch();
	~BaseSwitch() { }

	/// 更新
	void update() override;

	void restart() override;

	void action( const string_t& ) override;

	bool_t do_switch();
	bool_t do_break();

	bool_t turn_on();
	bool_t turn_off();

	void add_event_handler( const char_t*, const EventHandler& ) override;
	
}; // class Switch

} // namespace blue_sky
