#ifndef BLUE_SKY_BASE_SWITCH_H
#define BLUE_SKY_BASE_SWITCH_H

#include "ActiveObject.h"
#include <game/TimedCache.h>
#include <functional>
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
	enum State { OFF = 0, ON, BROKEN };

	typedef std::function< void() > EventHandler;
	typedef std::list< EventHandler > EventHandlerList;
	typedef std::map< string_t, EventHandlerList > EventHandlerMap;

	typedef game::TimedCache< const GameObject* > CntactObjectCache;
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

	bool_t do_switch();
	bool_t do_break();

	bool_t turn_on();
	bool_t turn_off();

	EventHandlerList& get_event_handler( const string_t& name ) { return event_handler_map_[ name ]; }
	
}; // class Switch

} // namespace blue_sky

#endif // BLUE_SKY_SWITCH_H
