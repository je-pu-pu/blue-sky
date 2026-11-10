#pragma once

#include <GameObject/ActiveObject.h>
#include <sstream>
#include <tuple>
#include <unordered_set>
#include <unordered_map>

namespace blue_sky
{

class ActiveObjectPhysics;
struct GameObjectContext;
namespace graphics { class GraphicsManager; }
}

namespace core::ecs { class EntityManager; }

namespace blue_sky
{

/**
 * 複数の ActiveObject を管理する Manager
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

	ActiveObjectPhysics* physics_;					///< 物理（GameMain が渡す。以前は GameMain のシングルトンから引いていた）
	graphics::GraphicsManager* graphics_manager_;	///< 描画（同上）
	const GameObjectContext* object_context_;		///< オブジェクトに渡すもの（物理の世界・音・経過時間。同上）
	core::ecs::EntityManager* entity_manager_;		///< オブジェクトの Entity（描画の部品）を作る先（同上。描画のデータは ecs::ObjectRenderComponent が持つ）

public:
	ActiveObjectManager( ActiveObjectPhysics* physics, graphics::GraphicsManager* graphics_manager, const GameObjectContext* object_context, core::ecs::EntityManager* entity_manager );
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

	/// 全オブジェクトの位置と描くかどうかを描画の部品へ写す（描く前に毎フレーム。GameMain がシーンを描く前に呼ぶ）
	void sync_render_components() const;

	/// 目標の位置に着いたか（残りが 0.1 未満か、このフレームで進める距離 speed × delta_time 以内）
	static bool is_target_location_reached( float_t distance, float_t speed, float_t delta_time );

	ActiveObjectList& active_object_list() { return active_object_list_; }
	const ActiveObjectList& active_object_list() const { return active_object_list_; }
};

} // namespace blue_sky
