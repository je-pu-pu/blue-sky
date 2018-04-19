#ifndef BLUE_SKY_ACTIVE_OBJECT_MANAGER_H
#define BLUE_SKY_ACTIVE_OBJECT_MANAGER_H

#include <GameObject/ActiveObject.h>
#include <sstream>
#include <tuple>
#include <set>
#include <map>

namespace blue_sky
{

class DrawingModelManager;
class ActiveObjectPhysics;

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
	typedef std::map< ActiveObject*, std::tuple< float_t, float_t > > TargetDirectionMap;
	typedef std::map< ActiveObject*, std::tuple< const ActiveObject*, float_t > > TargetDirectionObjectMap;

	typedef std::function< ActiveObject* () > CreateObjectFunction;
	

private:
	ActiveObjectList active_object_list_;
	ActiveObjectMap named_active_object_map_;
	TargetLocationMap target_location_map_;
	TargetDirectionMap target_direction_map_;
	TargetDirectionObjectMap target_direction_object_map_;

	std::map< string_t, CreateObjectFunction > object_creator_map_;

public:
	ActiveObjectManager();
	~ActiveObjectManager();

	void clear();

	void restart();

	void add_active_object( ActiveObject* active_object );
	void name_active_object( const string_t& name, ActiveObject* active_object );
	void set_target_location( ActiveObject* active_object, const Vector3& target_location, float_t speed );
	void set_target_direction( ActiveObject* active_object, float_t, float_t speed );
	void set_target_direction_object( ActiveObject* active_object, const ActiveObject* target_active_object, float_t speed );

	ActiveObject* create_object( const string_t& );
	ActiveObject* create_static_object( std::stringstream&, DrawingModelManager*, ActiveObjectPhysics* );
	ActiveObject* get_active_object( const string_t& name );

	void update();
	void render();

	ActiveObjectList& active_object_list() { return active_object_list_; }
	const ActiveObjectList& active_object_list() const { return active_object_list_; }
};

} // namespace blue_sky

#endif // BLUE_SKY_ACTIVE_OBJECT_MANAGER_H