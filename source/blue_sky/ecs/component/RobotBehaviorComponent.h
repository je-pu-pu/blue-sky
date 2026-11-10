#pragma once

#include <core/ecs/Component.h>
#include <core/ecs/type.h>
#include <core/graphics/Texture.h>
#include <core/type.h>

#include <optional>

class btRigidBody;

namespace blue_sky::ecs
{

/**
 * 敵の行動をまとめて持つ部品（以前の Robot クラス。段階 5 の決定 2 : 行動はまず 1 つの部品にまとめて ECS の中で動かし、1 つずつ切り出す）
 *
 * 状態の移り変わり（RobotBehaviorSystem）:
 * - 立つ（STAND）・回る（ROTATION）: 止まって（回るなら 1 フレームに 0.25 度回って）相手を探す。見えたら 注目（ATTENTION）へ
 * - 注目 : 相手の方を向いて 3 秒。まだ見えていれば 追う（CHASE）へ、見えなければ元の状態へ
 * - 追う : 相手（遠くて相手が動いていれば、相手の 3 m 先）へ向かう。10 秒たって見えなければ 回る へ。追っている間に相手に当たると、相手が傷つく
 * - 止まった（SHUTDOWN）: 立つ・回る間に相手か、こちらへ飛んでくる石に当たると止まる。止まるアニメーションの 60 フレーム目から当たりの応答が無くなる
 * - 浮く（FLOAT）: 始めの位置でふわふわと上下しながら、相手の方を向く
 * - 巡回（PATROL）: 巡回の地点（PatrolComponent）の今の地点へ秒速 0.1 で向かう（以前から次の地点へ進まない。ステージでは使っていない）
 *
 * 状態ごとのテクスチャ（ふだん・警戒・警報・やさしい）はハンドルで持ち、今の見た目（look）のものをモデルのシェーダーに設定する。
 * 巡回の地点は敵 AI のテストシーンと同じ PatrolComponent に持つ（やり直すと、始めの位置と 5 m 前の 2 つ）
 */
struct RobotBehaviorComponent : public core::ecs::Component
{
	enum class Mode
	{
		STAND = 0,
		ROTATION,
		PATROL,
		ATTENTION,
		FIND,		///< 以前から使っていない
		CHASE,
		SHUTDOWN,
		FLOAT,
	};

	/// 見た目（状態ごとのテクスチャのどれを使うか）
	enum class Look
	{
		NORMAL = 0,		///< ふだん（"robot"）
		WARNING,		///< 警戒（"robot-warn"）
		ALARM,			///< 警報（"robot-error"。Windows のヘッダが ERROR を定義しているので、この名前にした）
		TENDER,			///< やさしい（"robot-tender"。浮いているとき）
		MAX
	};

	Mode			mode = Mode::STAND;			///< 今の状態
	Mode			mode_backup = Mode::STAND;	///< 相手を見つけたときの状態（注目の後に見失ったら戻る）
	float_t			timer = 0.f;				///< 今の状態になってからの秒（注目・追う で使う）

	Look			look = Look::NORMAL;
	core::graphics::TextureHandle	textures[ static_cast< int >( Look::MAX ) ];	///< 見た目ごとのテクスチャ（作るときに名前で引く。以前は Texture* を 4 つ持っていた）

	/// 追う相手（いなければ無い）。相手の位置・速度・前・剛体は、相手の部品（Transform・PhysicsBody・SimpleObject）から読む
	/// （以前はプレイヤーが実装する窓口 ChaseTarget を持っていた。段階 6 でプレイヤーを ECS だけの物体にしたので部品を読む）
	std::optional< core::ecs::EntityId >	target_id;

	static constexpr float_t PatrolSpeed = 0.1f;			///< 巡回の速さ（以前の Robot::update_patrol）
	static constexpr float_t PatrolForwardDistance = 5.f;	///< やり直したときの 2 つ目の巡回の地点の前への距離
};

} // namespace blue_sky::ecs
