#pragma once

#include "GameObject.h"
#include <blue_sky/ConstantBuffers.h>
#include <blue_sky/ecs/component/TargetMoverComponent.h>
#include <common/exception.h>
#include <core/ecs/type.h>
#include <memory>

namespace core
{
	class AnimationPlayer;

	namespace graphics
	{
		class Shader;
		class Texture;
		class ConstantBuffer;
	}

	namespace ecs { class EntityManager; }
}

namespace blue_sky
{
	namespace graphics
	{
		class Model;
		class GraphicsManager;
	}

	namespace ecs
	{
		struct ObjectRenderComponent;
		struct AnimationComponent;
	}

/**
 * 行動するオブジェクト
 *
 * ecs::TargetMover は、目標地点・向きへの移動（ecs::MoveToTargetSystem）が位置を読み・速度を向け・着いたことを知らせる窓口。
 * 位置の正と剛体はまだこのクラスにあるので、ここで実装する（外からは使わせないので private で継承する）
 */
class alignas( 16 ) ActiveObject : public GameObject, private ecs::TargetMover
{
public:
	using Model				= graphics::Model;
	using Shader			= core::graphics::Shader;
	using Texture			= core::graphics::Texture;
	using AnimationPlayer	= core::AnimationPlayer;

private:
	Vector				front_;				///< 前
	Vector				right_;				///< 右

	Vector				start_location_;	///< スタート時の位置
	Vector				start_rotation_;	///< スタート時の回転 ( Degree )
	float_t				start_direction_degree_;	///< スタート時の向き ( Y Axis )

	/**
	 * 描画のデータ（モデル・定数バッファ・前フレームの world）とアニメーションは、Entity の部品が持つ
	 * （ObjectRenderComponent・AnimationComponent。以前はここに Model* の生ポインタ・定数バッファ・AnimationPlayer があった）。
	 * Entity は ActiveObjectManager に加えるとき（attach_entity）に作り、壊すときに消す
	 */
	core::ecs::EntityManager*	entity_manager_ = nullptr;		///< Entity を作った先（attach_entity で受け取る）
	core::ecs::EntityId			entity_id_ = 0;					///< 自分の Entity（has_entity_ が true のときだけ有効）
	bool						has_entity_ = false;
	Model*						pending_model_ = nullptr;		///< Entity を作る前に set_model されたモデル（attach_entity で部品へ移す）
	graphics::GraphicsManager*	graphics_manager_ = nullptr;	///< 定数バッファを作る先・モデルをハンドルから引く先（attach_entity で受け取る）

	bool				is_dead_;			///< 死亡フラグ
	float_t				flicker_scale_;		///< ゆらぎの大きさ

	float_t				direction_degree_;	///< 方向 ( Y Axis Degree )

	bool				is_mesh_visible_;	///< メッシュを描画するかどうか
	bool				is_line_visible_;	///< ラインを描画するか

	// ecs::TargetMover（MoveToTargetSystem が呼ぶ。以前 ActiveObjectManager::update が直接呼んでいたものへつなぐ）
	Vector get_target_mover_location() const override { return get_location(); }
	void move_toward_target_location( const Vector& target_location, float_t speed ) override { update_velocity_by_target_location( target_location, speed ); }
	void stop_at_target_location( const Vector& target_location ) override;
	void notify_arrive_at_target_location() override { on_arrive_at_target_location(); }
	void chase_target_direction_degree( float_t degree, float_t speed ) override { chase_direction_degree( degree, speed ); }
	void chase_target_direction_to( const Vector& location, float_t speed ) override { chase_direction_to( location, speed ); }

protected:
	void setup_animation_player();

	ecs::ObjectRenderComponent* get_render_component() const;
	ecs::AnimationComponent* get_animation_component() const;

	virtual void limit_velocity();

	constexpr float get_max_speed() const { return 20.f; }

public:
	ActiveObject();
	ActiveObject( const ActiveObject& o );
	virtual ~ActiveObject() override;

	virtual ActiveObject* clone() const { COMMON_THROW_EXCEPTION_MESSAGE( "clone() not implemented." ); }

	virtual void restart();

	virtual void update() override { }

	virtual void set_model( Model* m );
	Model* get_model();
	const Model* get_model() const;

	/**
	 * モデルとシェーダーを、このオブジェクトだけのものにする（メッシュ・線・アニメーションは共有のまま）
	 *
	 * create_object で作ったオブジェクトは、同じ種類のオブジェクトとモデル・シェーダーを共有している。
	 * このオブジェクトだけシェーダーのテクスチャやパラメータを変えたいときに、先に呼ぶ。
	 * まだ ActiveObjectManager に登録していない（複製する先が無い）ときやモデルが無いときは何もしない
	 *
	 * @return 自分だけのものにしたら true
	 */
	bool make_model_unique();

	virtual void action( const string_t& );

	virtual void set_flicker_scale( float_t s ) { flicker_scale_ = s; }
	virtual float_t get_flicker_scale() const { return flicker_scale_; }

	AnimationPlayer* get_animation_player();
	const AnimationPlayer* get_animation_player() const;

	void set_start_location( float_t, float_t, float_t );
	void set_start_rotation( float_t, float_t, float_t );
	void set_start_direction_degree( float_t );

	float_t get_direction_degree() const { return direction_degree_; }
	void set_direction_degree( float_t );
	void chase_direction_degree( float_t, float_t );
	void chase_direction_to( const Vector&, float_t );

	const Vector& get_front() const { return front_; }
	const Vector& get_right() const { return right_; }

	Vector& get_start_location() { return start_location_; }
	const Vector& get_start_location() const { return start_location_; }

	virtual void kill();
	
	bool is_dead() const { return is_dead_; }

	/// 描画されるかどうかを返す
	virtual bool is_visible() const { return ( is_mesh_visible() || is_line_visible() ); }

	virtual bool is_mesh_visible() const { return is_mesh_visible_; }
	virtual bool is_line_visible() const { return is_line_visible_; }

	void set_mesh_visible( bool v ) { is_mesh_visible_ = v; }
	void set_line_visible( bool v ) { is_line_visible_ = v; }

	/**
	 * 自分の Entity を作り、描画の部品（ObjectRenderComponent・TransformComponent）と定数バッファを用意する。
	 * ActiveObjectManager に加えるときに呼ばれる。既に Entity があれば何もしない（作り直さない）
	 *
	 * @param entity_manager Entity を作る先
	 * @param graphics_manager 定数バッファを作る先・モデルを引く先
	 */
	void attach_entity( core::ecs::EntityManager* entity_manager, graphics::GraphicsManager* graphics_manager );

	/// 自分の Entity を消す（部品も消える）。デストラクタからも呼ばれる。Entity が無い・もう消えているなら何もしない
	void detach_entity();

	bool has_entity() const { return has_entity_; }
	core::ecs::EntityId get_entity_id() const { return entity_id_; }

	/**
	 * 位置と描くかどうかを部品へ写す（描く前に毎フレーム。ActiveObjectManager::sync_render_components が呼ぶ）
	 *
	 * 橋渡しの間は位置（GameObject の Transform）と描くかどうか（派生が is_mesh_visible などを上書きする）の正はこのクラスにある。
	 * 段階 3 で位置の正を TransformComponent に移したら、位置は写さなくなる
	 */
	void sync_render_component() const;

	void play_animation( const char_t* name, bool force, bool loop );

	// virtual void render_line() = 0;

	static void* operator new ( size_t size )
	{
        return _aligned_malloc( size, 16 );
    }

	static void operator delete ( void* p )
	{
		_aligned_free( p );
	}

}; // class ActiveObject

#define DEFINE_ACTIVE_OBJECT_COMMON_FUNCTIONS \
	ActiveObject* clone() const override { return new std::remove_const< std::remove_reference< decltype( *this ) >::type >::type( * this ); }

} // namespace blue_sky
