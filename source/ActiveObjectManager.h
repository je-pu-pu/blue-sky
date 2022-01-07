#pragma once

#include <GameObject/ActiveObject.h>
#include <sstream>
#include <tuple>
#include <unordered_set>
#include <unordered_map>

namespace blue_sky
{

/**
 * ï°êîÇÃ ActiveObject Çä«óùÇ∑ÇÈ Manager
 *
 * @todo GameObjectManager Ç∆Ç∑ÇÈ
 */
class ActiveObjectManager
{
public:
	using ActiveObjectList			= std::unordered_set< ActiveObject* >;
	using ActiveObjectMap			= std::unordered_map< string_t, ActiveObject* >;
	using TargetLocationMap			= std::unordered_map< ActiveObject*, std::tuple< Vector, float_t > >;
	using TargetDirectionMap		= std::unordered_map< ActiveObject*, std::tuple< float_t, float_t > >;
	using TargetDirectionObjectMap	= std::unordered_map< ActiveObject*, std::tuple< const ActiveObject*, float_t > >;

	using CreateObjectFunction		= std::function< ActiveObject* () >;

private:
	ActiveObjectList active_object_list_;
	ActiveObjectMap named_active_object_map_;
	TargetLocationMap target_location_map_;
	TargetDirectionMap target_direction_map_;
	TargetDirectionObjectMap target_direction_object_map_;

	std::unordered_map< string_t, CreateObjectFunction > object_creator_map_;

public:
	ActiveObjectManager();
	~ActiveObjectManager();

	void clear();

	void restart();

	void add_active_object( ActiveObject* active_object );
	void name_active_object( const string_t& name, ActiveObject* active_object );
	void set_target_location( ActiveObject* active_object, const Vector& target_location, float_t speed );
	void set_target_direction( ActiveObject* active_object, float_t, float_t speed );
	void set_target_direction_object( ActiveObject* active_object, const ActiveObject* target_active_object, float_t speed );

	ActiveObject* create_object( const string_t& );
	ActiveObject* create_static_object( std::stringstream& );
	ActiveObject* get_active_object( const string_t& name );
	GameObject* get_nearest_object( const Vector& );

	ActiveObject* clone_object( const ActiveObject* );

	void update();

	ActiveObjectList& active_object_list() { return active_object_list_; }
	const ActiveObjectList& active_object_list() const { return active_object_list_; }
};

} // namespace blue_sky
