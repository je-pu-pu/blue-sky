#pragma once

#include <blue_sky/type.h>
#include <core/ecs/type.h>

#include <functional>

class btRigidBody;

namespace core::ecs
{
	class EntityManager;
	class Entity;
}

namespace blue_sky
{

class ActiveObject;

namespace graphics
{
	class GraphicsManager;
	class Model;
}

/**
 * ゲームの物体を EntityId で指す（Lua の "GameObject" 型・ステージのコマンドが物体を操作する窓口）
 *
 * 以前は ActiveObject* そのものを Lua に渡し、ステージのコマンドも名前で引いた ActiveObject* を操作していた。
 * ECS だけの物体（ActiveObject を持たない物体。段階 4 で移した種類）も同じように触れるよう、EntityId を包む。
 *
 * - ActiveObject を持つ物体（ActiveObjectLinkComponent がある）は、その ActiveObject のメンバ関数を呼ぶ（互換の層）
 * - 持たない物体は、部品を通して同じ決まりで動かす（ecs::EntityObject）
 *
 * 値で持ってよい（Entity が消えたら何もしない。is_alive で確かめられる）。EntityId は二度と使わない番号なので、別の物体を指すことは無い
 */
class GameObjectRef
{
public:
	using EntityManager		= core::ecs::EntityManager;
	using EntityId			= core::ecs::EntityId;
	using Model				= graphics::Model;
	using EventHandler		= std::function< void() >;

private:
	EntityManager*				entity_manager_ = nullptr;
	graphics::GraphicsManager*	graphics_manager_ = nullptr;	///< モデルをハンドルにする・引く先（無ければモデルを扱わない。テスト）
	EntityId					id_ = 0;

	core::ecs::Entity* find_entity() const;

	/// ECS だけの物体に、その物体ではできない操作をしたことを残す（黙って無視しない）
	void log_unsupported( const char* operation ) const;

public:
	GameObjectRef() = default;
	GameObjectRef( EntityManager* entity_manager, graphics::GraphicsManager* graphics_manager, EntityId id );

	EntityId get_entity_id() const { return id_; }

	/// 指している Entity がまだあるか
	bool is_alive() const;

	/// ActiveObject を持つ物体ならその ActiveObject（ECS だけの物体・消えた物体なら nullptr）
	ActiveObject* get_active_object() const;

	/// 剛体（無ければ nullptr。持ち主ではない）
	btRigidBody* get_rigid_body() const;

	core::Vector get_location() const;
	void set_location( const core::Vector& location );
	void set_location( float_t x, float_t y, float_t z ) { set_location( core::Vector( x, y, z ) ); }

	/// 向き（Y 軸の度。消えた物体なら 0）
	float_t get_direction_degree() const;
	void set_direction_degree( float_t degree );

	/// 速度（剛体が無い・消えた物体なら 0）
	core::Vector get_velocity() const;
	void set_velocity( const core::Vector& velocity );

	/// 今の速度で location に近づいているか（以前の GameObject::is_moving_to。敵が石に当たったときに使う）
	bool is_moving_to( const core::Vector& location ) const;

	void set_start_location( float_t x, float_t y, float_t z );
	void set_start_rotation( float_t x, float_t y, float_t z );
	void set_start_direction_degree( float_t degree );

	void set_mass( float_t mass );
	void set_gravity( const core::Vector& gravity );
	void set_friction( float_t friction );
	void set_angular_factor( const core::Vector& factor );
	void set_kinematic( bool is_kinematic );
	void set_no_contact_response( bool is_no_contact_response );

	void set_flicker_scale( float_t scale );

	void play_animation( const char_t* name, bool force, bool loop );
	void set_animation_speed( float_t speed );

	/// 固有のアクション（以前の ActiveObject::action。ECS だけの物体は "break_animation 0/1"・"set_visible メッシュ 線" を受け付ける）
	void action( const string_t& action );

	void kill();
	void restart();

	Model* get_model() const;
	void set_model( Model* model );

	/// モデルとシェーダーを、この物体だけのものにする（以前の ActiveObject::make_model_unique）
	bool make_model_unique();

	/// イベント（スイッチが入った など）の受け取り手を加える（イベントを持たない物体はログを残して何もしない）
	void add_event_handler( const char_t* name, const EventHandler& handler );

	/// 目標地点へ動かす（ecs::MoveToTargetSystem に頼む。着いたら止まる）。以前の ActiveObjectManager::set_target_location
	void set_target_location( const core::Vector& location, float_t speed );
	/// 決まった向きを追わせる
	void set_target_direction( float_t degree, float_t speed );
	/// ほかの物体の方向を追わせる
	void set_target_direction_to( const GameObjectRef& target, float_t speed );
};

} // namespace blue_sky
