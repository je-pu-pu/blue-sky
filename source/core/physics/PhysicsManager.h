#pragma once

#include <core/math/Vector.h>
#include <core/math/Transform.h>
#include <core/math/Quaternion.h>
#include <core/physics/RigidBody.h>
#include <core/ecs/type.h>
#include <memory>
#include <vector>
#include <functional>
#include <optional>

class btDynamicsWorld;
class btIDebugDraw;
class btRigidBody;
class btCollisionShape;
class btMotionState;

namespace core::physics
{

/**
 * 衝突グループ (ビットフラグ)
 */
namespace CollisionGroup
{
	constexpr short Default  = 1 << 0;  // 地面、建物など
	constexpr short Player   = 1 << 1;  // プレイヤー
	constexpr short Balloon  = 1 << 2;  // 風船
	constexpr short All      = 0x7FFF;  // 全て
}

/**
 * 剛体の生成パラメータ
 */
struct RigidBodyCreateInfo
{
	using Vector = math::Vector;
	using Transform = math::Transform;

	/// Bullet の剛体の摩擦の既定（btRigidBodyConstructionInfo の m_friction。本編の物体は以前からこの値）
	static constexpr float BulletDefaultFriction = 0.5f;

	RigidBodyShapeType shape_type = RigidBodyShapeType::Box;
	Vector shape_size = Vector( 0.5f, 0.5f, 0.5f );	///< 箱・円柱は半分の大きさ、カプセルは ( 半径, 円柱部の高さ, - )、球は ( 半径, -, - )
	float mass = 1.f;
	Transform transform;

	/// 剛体の中心のずれ（transform に右から掛ける。本編の物体は足元から中心までの高さ。無ければ掛けない）
	std::optional< Transform > offset;

	float friction = 0.1f;		///< 摩擦（ECS のテストシーンの既定 0.1。本編の物体は BulletDefaultFriction）

	/// 同じ種類・大きさの形を、ほかの剛体と共有する（本編の物体。形の持ち主は PhysicsManager で、clear で消える）。false ならこの剛体だけの形を作り、持ち主が持つ
	bool share_shape = false;

	/// 衝突グループとマスクを使わず、Bullet の既定（動く物は DefaultFilter・全部と当たる、動かない物は StaticFilter・動かない物とは当たらない）にする（本編の物体）
	bool use_default_collision_filter = false;

	/**
	 * 位置で動かす物（キネマティック。CF_KINEMATIC_OBJECT）として世界に入れる
	 *
	 * 物理では動かず（重力・速度・押されることで動かない）、使う側が set_world_transform で位置を書く。
	 * 世界に入れる前に印を付けるので、Bullet の既定の振り分けでは動かない物（StaticFilter）と同じ組になり、
	 * 動かない物・ほかのキネマティックな物との組を広い判定が作らない（動く物とは今までどおり当たりを調べる）
	 */
	bool is_kinematic = false;

	short collision_group = CollisionGroup::Default;  // 自身の衝突グループ
	short collision_mask = CollisionGroup::All;       // 衝突対象のマスク
};

/**
 * レイの結果（ray_test_closest）
 */
struct RayHit
{
	bool hit = false;						///< 当たったか
	float fraction = 1.f;					///< 当たった所の、始点から終点までの割合（当たらなければ 1）
	math::Vector point;						///< 当たった所（世界）
	math::Vector normal;					///< 当たった面の向き（世界）
	std::optional< ecs::EntityId > entity;	///< 当たった剛体の Entity の番号のタグ（地面・ステージの当たりの形・タグの無い剛体は std::nullopt）
};

/**
 * 剛体を数えるか決める関数（レイ・接触の判定）
 *
 * 剛体の Entity の番号のタグ（タグが無ければ std::nullopt）を受け取り、数えるなら true を返す
 */
using BodyFilter = std::function< bool( std::optional< ecs::EntityId > ) >;

/**
 * 接した組を受け取る関数（contact_test・for_each_penetrating_contact）
 *
 * 2 つの剛体の Entity の番号のタグ（タグが無ければ std::nullopt）を Bullet が返した順で受け取る
 */
using ContactCallback = std::function< void( std::optional< ecs::EntityId >, std::optional< ecs::EntityId > ) >;

/**
 * 剛体ハンドル（= 剛体の持ち主 RigidBody。ムーブだけできる）
 *
 * 以前は生ポインタ 3 つ（剛体・形・動きの状態）と user_pointer の組をコピーして持ち、destroy_rigid_body で消していた。
 * 今は持ち主が壊れる（部品が外れる・Entity が消える）と、世界から外して消える。PhysicsManager はこれより長く生きること
 */
using RigidBodyHandle = RigidBody;

/**
 * 物理マネージャー
 *
 * 物理シミュレーションを管理する。
 * Bullet Physics の詳細を隠蔽し、抽象的なインターフェースを提供する。
 *
 * 剛体の持ち主 :
 * - 物体の剛体（create_rigid_body）は、持ち主（RigidBody）を返す。受け取った側（物体の部品）が持ち、壊れるときに世界から外れる
 * - 地面・ステージの当たりの形（add_ground・load_obj）と、共有する形（RigidBodyCreateInfo::share_shape）は、これが持つ（clear で消す）
 * 持ち主はこれより先に壊れること。
 *
 * 以前は本編が別の世界（blue_sky::ActiveObjectPhysics。core の BulletPhysics を継承）を使い、Bullet の型（btRigidBody・
 * btCollisionWorld のコールバック）を本編のコードが直接扱っていた。今は本編も ECS のテストシーンもこの窓口を使う
 */
class PhysicsManager
{
public:
	using Vector = math::Vector;
	using Transform = math::Transform;

private:
	class Impl;
	std::unique_ptr< Impl > impl_;		///< 以前は生の new / delete

public:
	PhysicsManager();
	~PhysicsManager();

	// コピー禁止
	PhysicsManager( const PhysicsManager& ) = delete;
	PhysicsManager& operator=( const PhysicsManager& ) = delete;

	/**
	 * 剛体を生成してワールドに追加する
	 *
	 * @return 剛体の持ち主（捨てると、すぐに世界から外れて消える）
	 */
	RigidBodyHandle create_rigid_body( const RigidBodyCreateInfo& info );

	/**
	 * 剛体をワールドから削除して破棄する（handle.reset() と同じ。無効なら何もしない）
	 */
	void destroy_rigid_body( RigidBodyHandle& handle );

	/**
	 * ワールドにある剛体（衝突物）の数を返す（テスト・漏れの確認用）
	 */
	int get_rigid_body_count() const;

	/**
	 * 剛体の位置・回転を取得する
	 */
	void get_rigid_body_transform( const RigidBodyHandle& handle, Vector& position, math::Quaternion& rotation ) const;

	/**
	 * シミュレーションを1ステップ進める
	 */
	void step_simulation();
	void step_simulation( float time_step );

	/**
	 * 重力を設定する
	 */
	void set_gravity( const Vector& gravity );

	/**
	 * 重力を取得する
	 */
	Vector get_gravity() const;

	/**
	 * タイムステップを設定する
	 */
	void set_time_step( float time_step );

	/**
	 * タイムステップを取得する
	 */
	float get_time_step() const;

	/**
	 * デバッグ描画を設定する
	 */
	void set_debug_drawer( btIDebugDraw* drawer );

	/**
	 * 剛体の線形速度を設定する
	 */
	void set_rigid_body_linear_velocity( const RigidBodyHandle& handle, const Vector& velocity );

	/**
	 * 剛体の線形速度を取得する
	 */
	Vector get_rigid_body_linear_velocity( const RigidBodyHandle& handle ) const;

	/**
	 * 剛体の角速度係数を設定する (回転の制限)
	 * @param factor 各軸の係数 (0 = 回転なし, 1 = 通常)
	 */
	void set_rigid_body_angular_factor( const RigidBodyHandle& handle, const Vector& factor );

	/**
	 * 剛体にインパルスを適用する (ジャンプ用)
	 */
	void apply_impulse( const RigidBodyHandle& handle, const Vector& impulse );

	/**
	 * 剛体をアクティブ化する (スリープ解除)
	 */
	void activate_rigid_body( const RigidBodyHandle& handle );

	/**
	 * レイキャストを行う (接地判定用)
	 * @param from 開始点
	 * @param to 終点
	 * @return ヒットした場合 true
	 */
	bool ray_test( const Vector& from, const Vector& to ) const;

	/**
	 * レイキャストを行う (自分自身を除外)
	 * @param from 開始点
	 * @param to 終点
	 * @param exclude 除外する剛体
	 * @return ヒットした場合 true
	 */
	bool ray_test_excluding( const Vector& from, const Vector& to, const RigidBodyHandle& exclude ) const;

	/**
	 * 2つの剛体が接触しているかを調べる
	 * @param a 剛体A
	 * @param b 剛体B
	 * @return 接触している場合 true
	 */
	bool check_contact( const RigidBodyHandle& a, const RigidBodyHandle& b ) const;

	/**
	 * 指定した衝突グループ間の全ての衝突を処理する
	 * @param group_a 衝突グループA
	 * @param group_b 衝突グループB
	 * @param callback 衝突時に呼ばれるコールバック (group_a 側の Entity の番号, group_b 側の Entity の番号)。
	 *                 両方の剛体に Entity の番号のタグ（EntityTag.h）が付いている組だけを呼ぶ。
	 *                 以前は userPointer（RigidBodyComponent*）をそのまま渡し、受け取る側がキャストしていた
	 */
	void for_each_contact( short group_a, short group_b,
		const std::function< void( ecs::EntityId, ecs::EntityId ) >& callback ) const;

	// ---- 本編のための機能（以前の BulletPhysics・blue_sky::ActiveObjectPhysics から移した） ----

	/**
	 * これが持つ剛体・形・制約を消す（シーンの終わり）
	 *
	 * 物体の剛体は持ち主（物体の部品）が消すので、先に物体を壊しておくこと。残っていたら世界から外すだけで消さない
	 * （以前の BulletPhysics::clear と同じ）。重力は既定に戻す
	 */
	void clear();

	/**
	 * 地面（動かない箱）を足す（これが持つ。clear で消える。以前の BulletPhysics::add_ground_rigid_body）
	 *
	 * @param half_extents 箱の半分の大きさ。上の面が y = 0 になるように置く
	 */
	void add_ground( const Vector& half_extents );

	/**
	 * Wavefront OBJ ファイルの三角形を、動かない当たりの形として足す（これが持つ。clear で消える。以前の BulletPhysics::load_obj）
	 *
	 * 頂点は z を反転して読む（以前と同じ）。面は三角形だけを使う。
	 *
	 * @return ファイルを開けなければ false
	 * @throw common::exception_base 三角形が 1 つも無い
	 */
	bool load_obj( const char_t* file_name );

	/**
	 * 線分の始点に一番近い当たりを調べる（以前の blue_sky::ClosestNotMe・ClosestNotMeAndHim の仕組み）
	 *
	 * @param exclude_a 通り抜ける剛体（自分。nullptr なら無し）
	 * @param exclude_b 通り抜ける剛体（相手。nullptr なら無し）
	 * @param filter 当たる剛体か決める（タグを受け取り、当たるなら true。空なら全部に当たる）。exclude_a・exclude_b には呼ばない
	 */
	RayHit ray_test_closest( const Vector& from, const Vector& to, const RigidBody* exclude_a = nullptr, const RigidBody* exclude_b = nullptr, const BodyFilter& filter = { } ) const;

	/// 2 つの剛体が今接しているかを Bullet の接触の判定（contactPairTest）で調べる（以前の ActiveObjectPhysics::is_collision。どちらかが nullptr・空なら false）
	bool contact_pair_test( const RigidBody* a, const RigidBody* b ) const;

	/// 剛体 body と接している組を、見つけた順に callback に渡す（以前の ActiveObjectPhysics::collect_contacts_with。Bullet の contactTest）
	void contact_test( const RigidBody& body, const ContactCallback& callback ) const;

	/// 世界の中でめり込んでいる接点 1 つにつき 1 組を、callback に渡す（以前の ActiveObjectPhysics::collect_all_contacts。順も同じ）
	void for_each_penetrating_contact( const ContactCallback& callback ) const;

	/**
	 * 世界に入れない箱を置いて、何かに触れるかを調べる（以前のプレイヤーの壁・よじ登りの判定）
	 *
	 * @param transform 箱を置く物体の位置と回転
	 * @param offset_position 物体から見た箱の中心
	 * @param half_extents 箱の半分の大きさ
	 * @param exclude 数えない剛体（自分。nullptr なら無し）
	 * @param filter 数える剛体か決める（タグを受け取り、数えるなら true。空なら全部数える）。接点の両方の剛体（置いた箱も、タグ無しとして）に呼び、両方とも数えるときだけ触れたとする
	 */
	bool box_contact_test( const Transform& transform, const Vector& offset_position, const Vector& half_extents, const RigidBody* exclude = nullptr, const BodyFilter& filter = { } ) const;

	/// 箱の形の、余白を含めた半分の大きさ（Bullet の btBoxShape::getHalfExtentsWithMargin。プレイヤーの壁の判定の箱を置く高さに使う）
	static Vector get_box_half_extents_with_margin( const Vector& half_extents );

	/// これが持つ形の数（共有する形・ステージの当たりの形。テスト用）
	std::size_t get_owned_shape_count() const;

	/// これが持つ剛体の数（地面・ステージの当たりの形。シーンを片付けた後の漏れの確かめ用）
	std::size_t get_owned_rigid_body_count() const;

	/// 世界の様子の数（開発用の計測。BLUE_SKY_DEBUG_FRAME_STAGES のログに出す）
	struct Stats
	{
		int collision_objects = 0;		///< 世界にある剛体（衝突物）の数
		int active_objects = 0;			///< そのうち眠っていない（isActive）数
		int kinematic_objects = 0;		///< そのうちキネマティックな数
		int overlapping_pairs = 0;		///< 広い判定が作った組の数
		int manifolds = 0;				///< 狭い判定の接触の組（マニフォールド）の数
		int touching_manifolds = 0;		///< そのうち接点のある数
	};

	/// 世界の様子を数える（剛体の数だけ回る。開発用）
	Stats get_stats() const;

}; // class PhysicsManager

} // namespace core::physics
