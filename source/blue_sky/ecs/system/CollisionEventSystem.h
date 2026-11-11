#pragma once

#include <core/ecs/System.h>
#include <blue_sky/ecs/component/CollisionTraitsComponent.h>

#include <optional>
#include <vector>

namespace blue_sky::ecs
{

/**
 * 当たりのイベントを受け取る側（互換の層。今は ActiveObjectManager が、番号から ActiveObject* を引いて二重ディスパッチを呼ぶ）
 */
class CollisionEventHandler
{
public:
	/// 物体 id が地面（タグの無い剛体 : 地面・ステージの当たりの形・持ち主の消えた剛体）に当たった
	virtual void on_collide_with_ground( core::ecs::EntityId id ) = 0;

	/// 物体 a と物体 b が当たった（両方とも生きていて死んでいない）
	virtual void on_collide( core::ecs::EntityId a, core::ecs::EntityId b ) = 0;

protected:
	~CollisionEventHandler() = default;
};

/**
 * 当たった組を集め、振り分けて、受け取る側に順に渡す
 *
 * 以前は ActiveObjectPhysics の当たりのコールバックが、剛体の userPointer を ActiveObject* にキャストし、
 * その場で地面・死んだ物体を見分けて on_collide_with を呼んでいた。今は物理の世界（core::physics::PhysicsManager）が返す
 * 剛体の Entity の番号のタグを集め（add_contact）、dispatch で 1 つずつ振り分けて CollisionEventHandler に渡す。
 *
 * 振り分け（classify）は渡す直前に 1 つずつ行う（以前と同じく、先に渡した当たりで死んだ物体は、後の当たりを起こさない）。
 * 集めた順に渡す（以前は見つけた順にその場で呼んでいた。順は同じ）。
 *
 * 対象は当たりの性質の部品（CollisionTraitsComponent）を持つ Entity（ActiveObject は加えるときに付ける）。
 * EntityManager::update() では何もしない（集めて渡す時点は本編のシーンが決める。当たりを調べる順は以前と同じ : プレイヤー → ゴール → 全部）
 */
class CollisionEventSystem : public core::ecs::System< CollisionTraitsComponent >
{
public:
	/// 当たった組（剛体の Entity の番号のタグ。タグの無い剛体は std::nullopt）
	struct Contact
	{
		std::optional< core::ecs::EntityId > a;
		std::optional< core::ecs::EntityId > b;
	};

	/// 振り分けた結果
	enum class EventType
	{
		NONE,		///< 何も起こさない（両方とも地面・どちらかが死んでいる・消えた Entity のタグ・当たりの性質を持たない Entity）
		GROUND,		///< a が地面に当たった
		PAIR,		///< a と b が当たった
	};

	struct Event
	{
		EventType				type = EventType::NONE;
		core::ecs::EntityId		a = 0;
		core::ecs::EntityId		b = 0;
	};

private:
	std::vector< Contact > contact_list_;	///< 集めた組（集めた順）

	/// id の当たりの性質（Entity が消えた・持たないなら nullptr）
	const CollisionTraitsComponent* find_traits( core::ecs::EntityId id ) const;

public:
	CollisionEventSystem() = default;

	void update() override { }		///< 何もしない（上の説明のとおり）

	/// 当たった組を加える（PhysicsManager::contact_test・for_each_penetrating_contact が渡す剛体のタグ）
	void add_contact( std::optional< core::ecs::EntityId > a, std::optional< core::ecs::EntityId > b ) { contact_list_.push_back( { a, b } ); }
	void add_contact( const Contact& contact ) { contact_list_.push_back( contact ); }

	/**
	 * 集めた組を、集めた順に 1 つずつ振り分けて handler に渡し、一覧を空にする
	 *
	 * 渡した先で物体が死ぬ・加わることがある。振り分けは渡す直前に行う
	 */
	void dispatch( CollisionEventHandler& handler );

	/**
	 * 組を振り分ける（以前の ActiveObjectContactResultCallback::on_collide と同じ決まり）
	 *
	 * - 片方がタグ無し（地面）: もう片方が生きていて死んでいなければ GROUND
	 * - 両方がタグ付き : 両方とも生きていて死んでいなければ PAIR（a, b の順は集めた順のまま）
	 * - それ以外（両方タグ無し・死んでいる・消えた Entity・当たりの性質を持たない Entity）は NONE
	 *   （消えた Entity のタグは、以前なら消えた物を指すポインタだった所。何も起こさない）
	 */
	Event classify( const Contact& contact ) const;

	/// まだ渡していない組の数（テスト用）
	std::size_t debug_contact_count() const { return contact_list_.size(); }

	/**
	 * 剛体の当たりの性質を引く（レイの判定・壁の判定が使う）
	 *
	 * @param entity_manager 引く先（nullptr なら nullptr を返す）
	 * @param tag 剛体の Entity の番号のタグ（タグが無い・Entity が消えた・性質を持たないなら nullptr を返す）
	 */
	static const CollisionTraitsComponent* find_traits( const core::ecs::EntityManager* entity_manager, std::optional< core::ecs::EntityId > tag );

	/// 剛体にタグがあり、その Entity が消えているか（タグの無い剛体は false）
	static bool is_vanished( const core::ecs::EntityManager* entity_manager, std::optional< core::ecs::EntityId > tag );

	/**
	 * 足場・視線のレイが当たる剛体か（以前の ClosestNotMe : 死んだ物・通り抜けられる物・Entity が消えた物を通り抜け、
	 * 足場に含めないなら硬くない物も通り抜ける。タグの無い剛体（地面・ステージの当たりの形）には当たる）
	 */
	static bool is_ray_target( const core::ecs::EntityManager* entity_manager, std::optional< core::ecs::EntityId > tag, bool include_soft_footing );

	/// 壁・障害物として数える剛体か（以前のプレイヤーの WithoutMeContactResultCallback : Entity が消えた物と、塞がない物を数えない。タグの無い剛体は数える）
	static bool is_block( const core::ecs::EntityManager* entity_manager, std::optional< core::ecs::EntityId > tag );
};

} // namespace blue_sky::ecs
