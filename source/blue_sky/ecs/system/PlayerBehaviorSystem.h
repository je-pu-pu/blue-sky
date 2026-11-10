#pragma once

#include <core/ecs/System.h>
#include <core/ecs/component/TransformComponent.h>
#include <blue_sky/ecs/component/PlayerBehaviorComponent.h>

#include <optional>
#include <vector>

namespace core::sound { class SoundManager; }

namespace blue_sky
{
	class ActiveObjectPhysics;
	class GameObjectRef;
	namespace graphics { class GraphicsManager; }
}

namespace blue_sky::ecs
{

enum class PickupKind;

/**
 * プレイヤー（PlayerBehaviorComponent）を動かす（以前の Player クラスのメンバ関数。値と順は以前と同じ）
 *
 * - 毎フレームの更新（update_players。以前の Player::update）: 足場・跳べるか・前の壁・よじ登れるか・のぞき込めるか・投げられるかを調べ、
 *   風船・ロケットの動作を進め、線の色・落ちる音・着地・死・速さの上限・重力を決める
 * - 操作（step・jump・rocket・throw_stone など）: PlayerInputSystem が入力から呼ぶ
 * - 当たり（on_touch_*）: 当たりの互換の層（ActiveObjectManager::on_collide_entity_objects）が、相手の部品の組み合わせで呼ぶ
 *   （以前は GameObject の仮想関数 on_collide_with_balloon・on_pick_up など。プレイヤーだけが実装していた）
 *
 * 位置・速度・向きは EntityObject を通して部品（TransformComponent・PhysicsBodyComponent・SimpleObjectComponent）を読み書きする。
 * 足場・壁を調べるレイと箱は本編の物理の世界（ActiveObjectPhysics）で飛ばす（nullptr なら調べない。テスト）。
 *
 * EntityManager::update() では動かさない（本編のシーンは呼ばない）。ActiveObjectManager::update が、ほかの物体より先に update_players を呼ぶ
 * （以前は ActiveObject の一覧の先頭のプレイヤーの update が最初に呼ばれていた）
 */
class PlayerBehaviorSystem : public core::ecs::System< PlayerBehaviorComponent, core::ecs::TransformComponent >
{
public:
	using ActionMode = PlayerBehaviorComponent::ActionMode;
	using ItemType = PlayerBehaviorComponent::ItemType;

private:
	struct Self;	///< 1 人のプレイヤーを動かすときに使う部品と窓口（.cpp だけで定義する）

	const ActiveObjectPhysics*			physics_;			///< 足場・壁を調べる世界（nullptr なら調べない。テスト）
	graphics::GraphicsManager*			graphics_manager_;	///< 線の色を変えるモデルを引く先（nullptr なら変えない。テスト）
	core::sound::SoundManager*			sound_manager_;		///< 音を鳴らす先（nullptr なら鳴らさない。テスト）
	std::vector< core::ecs::EntityId >	id_list_;			///< 作った順に回すための作業用の一覧（毎フレーム作り直す。中身は使い回す）

	bool find_self( core::ecs::EntityId id, Self& self ) const;

	void play_sound( const Self&, const char_t* name, bool loop = false, bool force = true ) const;
	void stop_sound( const char_t* name ) const;
	void fade_out_sound( const char_t* name ) const;

	void update_on_footing( Self& );
	void update_jumpable( Self& );
	void update_jumping( Self& );
	void update_located_to_die( Self& );
	void update_facing_to_block( Self& );
	void update_can_clamber( Self& );
	void update_can_peer_down( Self& );
	void update_can_throw( Self& );
	void update_step_speed( Self& );
	void update_gravity( Self& );
	void limit_velocity( Self& );

	bool check_on_footing( const Self&, const core::Vector& from, float_t ray_length, bool include_soft_footing = false ) const;
	float_t get_footing_height( Self&, const core::Vector& from, bool include_soft_footing = false ) const;

	/// 箱が自分以外の壁・障害物に触れているか（offset は自分の位置・向きからの置き場所）
	bool is_box_touching_block( const Self&, const core::Vector& half, const core::Vector& offset ) const;

	bool can_running( const Self& ) const;
	bool is_falling( const Self& ) const;

	void set_action_mode( Self&, ActionMode );
	void finish_rocketing( Self& );
	void set_eye_depth( Self&, float_t );
	void kill( Self& );

public:
	PlayerBehaviorSystem( const ActiveObjectPhysics* physics, graphics::GraphicsManager* graphics_manager, core::sound::SoundManager* sound_manager );

	void update() override { }		///< 何もしない（上の説明のとおり）

	/// 全部のプレイヤーを動かす（作った順 = Entity の番号の順。以前の Player::update）
	void update_players( float_t delta_time, float_t scene_elapsed_time );

	/// 1 人のプレイヤーを動かす（プレイヤーでない・剛体が無いなら何もしない）
	void update_player( core::ecs::EntityId id, float_t delta_time, float_t scene_elapsed_time );

	/// やり直す（以前の Player::restart の、ActiveObject::restart の後の所。EntityObject::restart が呼ぶ）
	void restart( core::ecs::EntityId id );

	/// プレイヤーの部品（プレイヤーでなければ nullptr）
	PlayerBehaviorComponent* find_player( core::ecs::EntityId id ) const;

	// 操作（PlayerInputSystem が入力から呼ぶ。以前の Player の同じ名前の関数）
	void step( core::ecs::EntityId id, float_t s );
	void side_step( core::ecs::EntityId id, float_t s );
	void ladder_step( core::ecs::EntityId id, float_t s );
	void stop( core::ecs::EntityId id );
	void stop_ladder_step( core::ecs::EntityId id );
	void jump( core::ecs::EntityId id );
	void clamber( core::ecs::EntityId id );
	void stop_clamber( core::ecs::EntityId id );
	void add_direction_degree( core::ecs::EntityId id, float_t d );

	/// 向きを変える（force でなければ、ロケットで飛んでいる間は変えない）
	void set_direction_degree( core::ecs::EntityId id, float_t d, bool force = false );

	void set_pitch( core::ecs::EntityId id, float_t pitch );
	void add_eye_depth( core::ecs::EntityId id, float_t d );
	void rocket( core::ecs::EntityId id, const core::Vector& direction );
	void throw_stone( core::ecs::EntityId id, const core::Vector& direction );
	void select_prev_item( core::ecs::EntityId id );
	void select_next_item( core::ecs::EntityId id );
	void switch_scope_mode( core::ecs::EntityId id );

	/// 揺らぎ始める（ステージの start_flickering。以前の Player::start_flickering）
	void start_flickering( core::ecs::EntityId id );

	/// 死なせる（以前の Player::kill。死んでいれば何もしない。EntityObject::kill（スクリプトの kill）も、プレイヤーならこれを呼ぶ）
	void kill( core::ecs::EntityId id );

	// 当たり（当たりの互換の層が、相手の部品の組み合わせで呼ぶ。以前の Player の on_collide_with_* と同じ）

	/// 風船に触れた : つかむ（揺らぎ中はつかまない）。つかんだら true
	bool on_touch_balloon( core::ecs::EntityId id, core::ecs::EntityId balloon );

	/// 石に当たった : 拾う（いつも true。拾われた石は互換の層が死なせる）
	bool on_touch_stone( core::ecs::EntityId id, const GameObjectRef& stone );

	/// 拾う物に当たった : メダル・ロケットを持つ（いつも true。拾われた物は互換の層が死なせる）
	bool on_touch_pickup( core::ecs::EntityId id, PickupKind kind );

	/// 敵に当たった : 追っている敵なら傷つく（いつも true = 敵は立つ・回る間なら止まる）
	bool on_touch_robot( core::ecs::EntityId id, bool is_chasing );

	/// 梯子に触れた : 梯子につかまる（いつも true = 梯子が触れた音を鳴らす）
	bool on_touch_ladder( core::ecs::EntityId id );

	/// 動かない物に当たった : ロケットで飛んでいれば止まる
	void on_touch_static_object( core::ecs::EntityId id );

	// 問い合わせ（プレイヤーでなければ false）
	bool is_falling( core::ecs::EntityId id ) const;
	bool is_falling_to_die( core::ecs::EntityId id ) const;
	bool is_dead( core::ecs::EntityId id ) const;

	/// 持っている風船を置く位置（手元。以前の Player::get_balloon_hold_location。BalloonHoldSystem が読む）
	static core::Vector get_balloon_hold_location( core::ecs::Entity& player );
};

} // namespace blue_sky::ecs
