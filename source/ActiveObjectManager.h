#ifndef BLUE_SKY_ACTIVE_OBJECT_MANAGER_H
#define BLUE_SKY_ACTIVE_OBJECT_MANAGER_H

#include "ActiveObject.h"
#include <tuple>
#include <set>
#include <map>

namespace blue_sky
{

/**
 * ï°êîÇÃ ActiveObject Çä«óùÇ∑ÇÈ Manager
 *
 */
class ActiveObjectManager
{
public:
	typedef GameObject::Vector3 Vector3;

	typedef std::set< ActiveObject* > ActiveObjectList;
	typedef std::map< string_t, ActiveObject* > ActiveObjectMap;
	typedef std::map< ActiveObject*, std::tuple< Vector3, float_t > > TargetLocationMap;

private:
	ActiveObjectList active_object_list_;
	ActiveObjectMap named_active_object_map_;
	TargetLocationMap target_location_map_;

public:
	ActiveObjectManager();
	~ActiveObjectManager();

	void clear();

	void restart();

	void add_active_object( ActiveObject* active_object );
	void name_active_object( const string_t& name, ActiveObject* active_object );
	void set_target_location( ActiveObject* active_object, const Vector3& target_location, float_t speed );

	ActiveObject* get_active_object( const string_t& name );

	void update();
	void render();

	ActiveObjectList& active_object_list() { return active_object_list_; }
	const ActiveObjectList& active_object_list() const { return active_object_list_; }
};

} // namespace blue_sky

#endif // BLUE_SKY_ACTIVE_OBJECT_MANAGER_H