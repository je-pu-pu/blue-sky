#ifndef BLUE_SKY_ACTIVE_OBJECT_MANAGER_H
#define BLUE_SKY_ACTIVE_OBJECT_MANAGER_H

#include <set>

namespace blue_sky
{

class ActiveObject;

/**
 * ï°êîÇÃ ActiveObject Çä«óùÇ∑ÇÈ Manager
 *
 */
class ActiveObjectManager
{
public:
	typedef std::set< ActiveObject* > ActiveObjectList;

private:
	ActiveObjectList active_object_list_;

public:
	ActiveObjectManager();
	~ActiveObjectManager();

	void clear();

	void add_active_object( ActiveObject* );

	void update();

	ActiveObjectList& active_object_list() { return active_object_list_; }
	const ActiveObjectList& active_object_list() const { return active_object_list_; }
};

} // namespace blue_sky

#endif // BLUE_SKY_ACTIVE_OBJECT_MANAGER_H