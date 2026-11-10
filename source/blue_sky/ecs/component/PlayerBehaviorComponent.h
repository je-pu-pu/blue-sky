#pragma once

#include <blue_sky/GameObjectRef.h>
#include <core/ecs/Component.h>
#include <core/ecs/type.h>
#include <core/type.h>

#include <list>
#include <optional>

namespace blue_sky::ecs
{

/**
 * プレイヤーの行動をまとめて持つ部品（以前の Player クラスのメンバ変数と定数）
 *
 * 決定（doc/ECS移行計画.md 5 章の 2）のとおり、プレイヤーは当面、行動をまとめて持つこの部品で ECS の中で動かし、1 つずつ切り出す。
 * 振る舞いは PlayerBehaviorSystem（以前の Player のメンバ関数と同じ値・順）、入力からの操作は PlayerInputSystem。
 * 位置の正は TransformComponent、剛体は PhysicsBodyComponent、向き・やり直しの位置は SimpleObjectComponent（ほかの物体と同じ）。
 *
 * 風船（BalloonHoldSystem）・敵（RobotBehaviorSystem）は、以前はプレイヤーが実装する窓口（BalloonHolder・ChaseTarget）を通して読んでいた。
 * 今はこの部品と、位置・向き・剛体の部品を読む
 */
struct PlayerBehaviorComponent : public core::ecs::Component
{
	enum class ActionMode
	{
		NONE = 0,
		BALLOON,
		ROCKET,
		UMBRELLA,
		SCOPE,
	};

	enum class ItemType
	{
		NONE = -1,
		ROCKET = 0,
		UMBRELLA,
		STONE,
		SCOPE,
		MAX
	};

	using StoneList = std::list< GameObjectRef >;	///< 持っている石（EntityId を包む窓口）

	bool		is_on_footing = false;			///< 現在足場上フラグ
	bool		is_jumping = false;				///< 現在ジャンプ中フラグ
	bool		is_jumpable = false;			///< 現在ジャンプ可能フラグ
	bool		is_clambering = false;			///< 現在よじ登り中フラグ

	bool		is_located_on_die = false;		///< 現在死亡へ向けて落下中フラグ
	bool		is_located_on_safe = false;		///< そのまま落下すると風船またはゴールと接触するフラグ

	bool		is_on_ladder = false;			///< 梯子と接触中フラグ（毎フレームの終わりに消し、梯子に触れると立つ）
	bool		is_facing_to_block = false;		///< 障害物の方を向いて接触中フラグ
	bool		can_clamber = false;			///< よじ登り可能フラグ
	bool		can_peer_down = false;			///< のぞき込み可能フラグ
	bool		can_throw = false;				///< 石投げ可能フラグ

	bool		is_flickering = false;			///< 現在揺らぎ中フラグ

	int			step_count = 0;					///< 移動カウンタ
	float_t		step_speed = 0.25f;				///< 移動速度（始めは歩く最高の速さ）

	ActionMode	action_mode = ActionMode::NONE;	///< 現在のアクションのモード
	float_t		action_timer = 0.f;				///< 現在のアクションが開始してからの経過時間
	core::Vector action_base_position = core::Vector::Zero;	///< 現在のアクションの基底位置
	bool		is_action_pre_finish = false;	///< 現在のアクションがもうすぐ終わるフラグ
	int_t		balloon_sequence_count = 0;		///< 現在の連続取得バルーンの数 ( 0 .. 7 )
	int_t		balloon_sound_request = 0;		///< 風船をつかんだときに鳴らす音の番号（シーンが取り出す。0 なら無し）

	float_t		uncontrollable_timer = 0.f;		///< 制御不能タイマー

	float_t		pitch = 0.f;					///< 上下の向き ( 真下 : -1.f / 真上 : 1.f )

	float_t		eye_height = 1.5f;				///< 目の高さ
	float_t		eye_depth = 0.f;				///< 目の奥行きオフセット ( m ) ( 身を乗り出した時に増える ) ( 0.f .. 1.f )

	bool		has_medal = false;				///< メダル保持フラグ

	int			item_count[ static_cast< int >( ItemType::MAX ) ] = { 0 };	///< 各アイテムの個数
	ItemType	selected_item_type = ItemType::NONE;	///< 現在選択中のアイテム

	float_t		last_footing_height = 0.f;		///< 前回の足場の高さ ( バルーン含む )

	std::optional< core::ecs::EntityId >	balloon;	///< 現在持っている風船（風船の動作が終わっても、次の風船をつかむかやり直すまで覚えている）
	StoneList	stone_list;						///< 現在持っている石のリスト

	int			hp = 1;							///< HP

	// 定数（以前の Player の get_* と同じ値）
	static constexpr float_t CollisionWidth = 0.5f;
	static constexpr float_t CollisionHeight = 1.75f;
	static constexpr float_t CollisionDepth = 0.5f;

	static constexpr float_t HeightToDie = 15.f;
	static constexpr float_t BalloonActionLength = 5.f;

	static constexpr float_t MinWalkStepSpeed = 0.1f;
	static constexpr float_t MaxWalkStepSpeed = 0.25f;
	static constexpr float_t MaxRunStepSpeed = 0.5f;

	static constexpr float_t SideStepSpeed = 0.25f;
	static constexpr float_t LadderStepSpeed = 2.f;

	static constexpr float_t MaxRunVelocity = 4.f;

	static constexpr float_t RocketInitialVelocity = 10.f;
	static constexpr float_t RocketActionLength = 10.f;
	static constexpr float_t RocketTimeout = 2.5f;
	static constexpr float_t RocketPreFinishRatio = 0.8f;

	static constexpr float_t BalloonMaxVelocity = 3.f;
	static constexpr float_t BalloonChaseSpeedDown = 0.5f;
	static constexpr float_t BalloonChaseSpeedUp = 0.25f;
	static constexpr float_t BalloonPreFinishRatio = 0.75f;

	static constexpr float_t JumpVelocity = 4.f;
	static constexpr float_t SuperJumpVelocity = 20.f;
	static constexpr float_t ClamberVelocity = 2.f;
	static constexpr float_t PlayerMass = 50.f;
	static constexpr float_t StoneThrowSpeed = 8.f;
	static constexpr float_t KnockbackSpeed = 10.f;
	static constexpr float_t KnockbackYVelocity = 2.5f;

	static constexpr float_t VelocityDampingNormal = 0.9f;
	static constexpr float_t VelocityDampingJumping = 0.95f;
	static constexpr float_t VelocityDampingLadder = 0.5f;
	static constexpr float_t MaxSpeed = 20.f;				///< 横（x・z）の速さの上限（以前の ActiveObject::get_max_speed）

	static constexpr float_t DefaultEyeHeight = 1.5f;
	static constexpr float_t EyeDepthDecay = 0.9f;
	static constexpr float_t DeadEyeDecay = 0.95f;
	static constexpr float_t DeadEyeMinHeight = 0.15f;
	static constexpr float_t MinHeightForPeer = 5.f;

	static constexpr float_t UncontrollableDuration = 1.5f;

	static constexpr float_t JumpVelocityThreshold = 3.f;
	static constexpr float_t LandingVelocityThreshold = 2.f;
	static constexpr float_t FallSoundVelocity = -7.5f;
	static constexpr float_t FallingVelocityThreshold = -1.f;

	static constexpr int StepCountToRun = 150;
	static constexpr float_t StepSpeedStopDecay = 0.5f;
	static constexpr float_t StepSpeedChaseRate = 0.01f;
	static constexpr float_t RunSpeedChaseRate = 0.001f;
	static constexpr float_t MinVelocityForRun = 1.f;
	static constexpr int MaxStonesForRunning = 3;

	static constexpr float_t MaxHeightDropForClamber = 5.f;

	// 状態だけで決まる問い合わせ（以前の Player の同じ名前の関数）
	bool is_walking() const { return step_speed > MinWalkStepSpeed; }
	bool is_running() const { return step_speed > MaxWalkStepSpeed; }
	bool is_uncontrollable() const { return uncontrollable_timer > 0.f; }
	bool is_rocketing() const { return action_mode == ActionMode::ROCKET; }
	bool is_ladder_step_only() const { return is_on_ladder && ! is_on_footing; }

	int get_item_count( ItemType type ) const
	{
		if ( type <= ItemType::NONE ) return 0;
		if ( type >= ItemType::MAX ) return 0;

		return item_count[ static_cast< int >( type ) ];
	}

	/// 風船の音の番号を取り出す（取り出すと 0 に戻る。以前の Player::pop_balloon_sound_request）
	int_t pop_balloon_sound_request() { const int_t r = balloon_sound_request; balloon_sound_request = 0; return r; }
};

} // namespace blue_sky::ecs
