#pragma once

#include <GameObject/ActiveObject.h>
#include <blue_sky/ecs/system/CollisionEventSystem.h>
#include <sstream>
#include <unordered_map>
#include <vector>

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
 *
 * 当たりのイベントの互換の層（ecs::CollisionEventHandler）も受け持つ。ecs::CollisionEventSystem が Entity の番号で渡す当たりを、
 * 番号から ActiveObject* を引いて、今の二重ディスパッチ（GameObject::on_collide_with）に渡す
 */
class ActiveObjectManager : public ecs::CollisionEventHandler
{
public:
	/// 加えた順（作った順）に並ぶ一覧。更新・スイッチの処理の順が起動ごとに変わらないように
	/// （以前は unordered_set< ActiveObject* > で、アドレスの順になり起動ごとに変わっていた）
	using ActiveObjectList			= std::vector< ActiveObject* >;
	using ActiveObjectMap			= std::unordered_map< string_t, ActiveObject* >;

	using CreateObjectFunction		= std::function< ActiveObject* () >;

private:
	ActiveObjectList active_object_list_;
	ActiveObjectMap named_active_object_map_;

	// 目標地点・向きは Entity の部品（ecs::TargetLocationComponent・TargetDirectionComponent）が持ち、ecs::MoveToTargetSystem が動かす
	// （以前はここに ActiveObject* を鍵にした 3 つの unordered_map があった）

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

	/// 目標地点へ動かす（ステージのコマンドの窓口。中身は ecs::MoveToTargetSystem に頼む。着いたら on_arrive_at_target_location が呼ばれる）
	void set_target_location( ActiveObject* active_object, const Vector& target_location, float_t speed );
	/// 決まった向きを追わせる（同上）
	void set_target_direction( ActiveObject* active_object, float_t, float_t speed );
	/// ほかの物体の方向を追わせる（同上）
	void set_target_direction_object( ActiveObject* active_object, const ActiveObject* target_active_object, float_t speed );

	ActiveObject* create_object( const string_t& );
	ActiveObject* create_static_object( std::stringstream& );
	ActiveObject* get_active_object( const string_t& name );
	GameObject* get_nearest_object( const Vector& );

	ActiveObject* clone_object( const ActiveObject* );

	void update();

	/**
	 * a と接している物体との当たりを起こす（以前の ActiveObjectPhysics::check_collision_with）
	 *
	 * 接した組を ecs::CollisionEventSystem に集め、振り分けて、互換の層（下の on_collide_*）が on_collide_with を呼ぶ
	 */
	void check_collision_with( ActiveObject* a );

	/// 世界の中で接している物体どうしの当たりを全部起こす（以前の ActiveObjectPhysics::check_collision_all）
	void check_collision_all();

	/// Entity の番号から物体を引く（無い・消えた・ActiveObject でないなら nullptr）
	ActiveObject* find_active_object( core::ecs::EntityId id ) const;

	// ecs::CollisionEventHandler（互換の層）
	void on_collide_with_ground( core::ecs::EntityId id ) override;
	void on_collide( core::ecs::EntityId a, core::ecs::EntityId b ) override;

	/// 全オブジェクトの位置と描くかどうかを描画の部品へ写す（描く前に毎フレーム。GameMain がシーンを描く前に呼ぶ）
	void sync_render_components() const;

	ActiveObjectList& active_object_list() { return active_object_list_; }
	const ActiveObjectList& active_object_list() const { return active_object_list_; }
};

} // namespace blue_sky
