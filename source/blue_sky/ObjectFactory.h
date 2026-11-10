#pragma once

#include <blue_sky/GameObjectRef.h>
#include <blue_sky/type.h>
#include <core/ecs/type.h>

#include <optional>
#include <sstream>
#include <unordered_map>
#include <vector>

namespace core::ecs { class EntityManager; }

namespace blue_sky
{

class ActiveObject;
class ActiveObjectManager;
class ActiveObjectPhysics;

namespace graphics
{
	class GraphicsManager;
	class Model;
}

/**
 * ゲームの物体を種類の名前から作り、名前で引けるようにする（作った物体は EntityId で指す）
 *
 * 以前はばらばらにあった「名前 → 作り方」をここへ集めていく（段階 4。移した種類から）:
 *   - ActiveObjectManager の object_creator_map_（"static" をここへ移した。ほかの種類はまだ ActiveObjectManager が作る）
 *   - GameMain::create_object（スクリプトの create_object。剛体は箱・モデルは種類の名前で読む）
 *   - ステージの object・static-object・dynamic-object・translation-object 行と game_object.create（ActiveObjectManager::create_static_object・TranslationObject）
 *   - 物体の名前の表（以前は ActiveObjectManager の named_active_object_map_ が ActiveObject* を持っていた。今は EntityId を持つ）
 *
 * 動かない物（"static"・ステージの object 行）と動く床（translation-object）は、ActiveObject を持たない ECS だけの物体として作る
 * （部品 : Transform・PhysicsBody・TargetMover（mover は nullptr）・CollisionTraits・StaticObject・SimpleObject・ObjectRender。動く床は Translation も）。
 * まだ ActiveObject の種類は ActiveObjectManager が作り、ここは EntityId を返す。
 *
 * ECS だけの物体の寿命は、ここが持つ（clear で Entity を消す。剛体の持ち主も部品なので一緒に消える）。
 * clear は ActiveObjectManager::clear も呼ぶ（物体の片付けの入口を 1 つにする）
 */
class ObjectFactory
{
public:
	using EntityId = core::ecs::EntityId;
	using Model = graphics::Model;

	/// 剛体の形
	enum class Shape
	{
		NONE,		///< 剛体を持たない（描くだけ）
		BOX,
		CYLINDER,
	};

	/// 動かない物を作るときの決まり
	struct StaticObjectSpec
	{
		string_t		kind;							///< 種類の名前（モデルの名前にも使う）
		core::Vector	size = core::Vector::Zero;		///< 当たりの大きさ（幅・高さ・奥行き）
		float_t			mass = 0.f;						///< 質量（0 なら動かない）
		Shape			shape = Shape::BOX;
		core::Vector	location = core::Vector::Zero;	///< やり直したときの位置
		core::Vector	rotation = core::Vector::Zero;	///< やり直したときの回転（度）
		float_t			direction_degree = 0.f;			///< やり直したときの向き（度）
		bool			load_model = true;				///< 種類の名前のモデルを読むか（後で別のモデルを設定するなら false）
	};

private:
	core::ecs::EntityManager*	entity_manager_;
	ActiveObjectManager*		active_object_manager_;		///< まだ ActiveObject の種類を作る先（nullptr ならその種類は作れない。テスト）
	ActiveObjectPhysics*		physics_;					///< 剛体を作る世界（nullptr なら剛体を作らない。テスト）
	graphics::GraphicsManager*	graphics_manager_;			///< モデル・定数バッファを作る先（nullptr ならモデル・定数バッファを用意しない。テスト）

	std::vector< EntityId >							entity_object_list_;	///< ここが作った ECS だけの物体（作った順）
	std::unordered_map< string_t, EntityId >		named_object_map_;		///< 名前 → 物体

	/// 物体を扱う System を用意する（シーンの切り替えで EntityManager が片付けるので、無ければ足す。あれば何もしない）
	void setup_systems();

public:
	ObjectFactory( core::ecs::EntityManager* entity_manager, ActiveObjectManager* active_object_manager, ActiveObjectPhysics* physics, graphics::GraphicsManager* graphics_manager );

	/**
	 * 種類の名前から物体を作る（スクリプトの create_object。以前の GameMain::create_object）
	 *
	 * 剛体は当たりの大きさの箱、モデルは種類の名前で読む。位置・向きは作った後に設定する（やり直しはしない）
	 *
	 * @return 作った物体（知らない種類ならログを残して std::nullopt）
	 */
	std::optional< EntityId > create_object( const string_t& kind );

	/// まだ ActiveObject の種類を作る（create_object の中身。GameMain::create_object・シーンが ActiveObject* を必要とする所が使う）
	ActiveObject* create_active_object( const string_t& kind );

	/**
	 * ステージの object 行・game_object.create から動かない物を作る（以前の ActiveObjectManager::create_static_object）
	 *
	 * 書き方 : 種類 x y z rx ry rz（大きさ・質量は種類で決まる。soda-can-1 だけ円柱）。やり直しはしない（ステージを始めるときにまとめてやり直す）
	 */
	EntityId create_static_object( std::stringstream& ss );

	/// 決まりに従って動かない物を作る（create_static_object の中身。テストでも使う）
	EntityId create_static_object( const StaticObjectSpec& spec );

	/// ステージの translation-object 行から動く床を作る（以前の TranslationObject。5 x 5 x 5 の箱・モデルは box-5x5x5）
	EntityId create_translation_object( const core::Vector& location, float_t width, float_t height, float_t depth, float_t speed );

	/// 剛体を持たない、描くだけの動かない物を作る（DebugScene の町並み。以前は ActiveObjectManager::create_object( "static" ) で剛体なし）
	EntityId create_model_object( Model* model );

	/// 名前を付ける（同じ名前は後の物体に付け替わる。以前と同じ）
	void name( const string_t& name, EntityId id );

	/// 名前で引く（無い・消えた物体なら std::nullopt）
	std::optional< EntityId > find( const string_t& name ) const;

	/// 物体を指す窓口を作る
	GameObjectRef get_ref( EntityId id ) const;

	/// 名前で引いて窓口を作る（無ければ std::nullopt）
	std::optional< GameObjectRef > find_ref( const string_t& name ) const;

	/// 全部の物体をやり直す（ActiveObject → ECS だけの物体の順。以前の ActiveObjectManager::restart）
	void restart();

	/// 全部の物体を片付ける（ActiveObject と ECS だけの物体を壊し、名前を忘れる）。物理の世界を片付ける前に呼ぶこと
	void clear();

	/// ここが作った ECS だけの物体（作った順）
	const std::vector< EntityId >& get_entity_object_list() const { return entity_object_list_; }
};

} // namespace blue_sky
